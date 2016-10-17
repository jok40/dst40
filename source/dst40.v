/******************************************************************************

  Модуль DST40.

  Версия 3: с развёрнутым циклом хэширования, преобразованным в 64-тактовый
            конвеер.

  1. Интерфейс с пользователем реализован на стороне HPS.

  2. Вторая проверка ключа с другими запросом/ответом запускается программно
     процессором HPS.
     
  3. Флаг прерывания IRQ0 взводится в момент взведения любого из флагов -
     ключ найден или ключ не найден, а сбрасывается записью в любой
     Avalon-регистр.

  4. HPS пишет/читает регистры с исходными данными через мост Avalon MM,
     подключенный к мосту HPS-FPGA шириной 64 бита.
     Адресация регистров выполняется блоками по 8 байт на 1 адрес.
     При чтении регистров неиспользуемые старшие биты зануляются,
     что упрощает дальнейшее использование данных в программе.

     Адресная карта:

     0 - challenge                ( 40 бит,      Чтение/Запись )  Запрос
     1 - response                 ( 24 бита,     Чтение/Запись )  Ответ
     2 - start_key                ( 40 бит,      Чтение/Запись )  Ключ, с которого начинать поиск
     3 - run                      (  1 бит,      Чтение/Запись )  Флаг запуска поиска
     4 - флаги:
         бит 0 - key_found_w      (       1 бит, Только чтение )  Флаг "ключ найден"
         бит 8 - key_not_found_w  (       1 бит, Только чтение )  Флаг "ключ не найден"
     5 - key                      ( 40-L2NK бит, Только чтение )  Найденный ключ (младшие биты)
     6 - kernels                  (      NK бит, Только чтение )  Биты ядер, нашедших ключ

******************************************************************************/

module dst40
(
  input         FPGA_CLK1_50,                                   // Такты 50 МГц
  input   [1:0] KEY,                                            // Кнопки
  input   [3:0] SW,                                             // Переключатели
  output  [7:0] LED,                                            // Светодиоды
  inout  [35:0] GPIO_0,                                         // Ножки GPIO0
  inout  [35:0] GPIO_1,                                         // Ножки GPIO1
  inout  [14:0] ARDUINO_IO,                                     // Ножки Arduino

  // HPS

  inout         HPS_CONV_USB_N,

  output [14:0] HPS_DDR3_ADDR,
  output  [2:0] HPS_DDR3_BA,
  output        HPS_DDR3_CAS_N,
  output        HPS_DDR3_CKE,
  output        HPS_DDR3_CK_N,
  output        HPS_DDR3_CK_P,
  output        HPS_DDR3_CS_N,
  output  [3:0] HPS_DDR3_DM,
  inout  [31:0] HPS_DDR3_DQ,
  inout   [3:0] HPS_DDR3_DQS_N,
  inout   [3:0] HPS_DDR3_DQS_P,
  output        HPS_DDR3_ODT,
  output        HPS_DDR3_RAS_N,
  output        HPS_DDR3_RESET_N,
  input         HPS_DDR3_RZQ,
  output        HPS_DDR3_WE_N,

  output        HPS_ENET_GTX_CLK,
  inout         HPS_ENET_INT_N,
  output        HPS_ENET_MDC,
  inout         HPS_ENET_MDIO,
  input         HPS_ENET_RX_CLK,
  input   [3:0] HPS_ENET_RX_DATA,
  input         HPS_ENET_RX_DV,
  output  [3:0] HPS_ENET_TX_DATA,
  output        HPS_ENET_TX_EN,

  inout         HPS_GSENSOR_INT,

  inout         HPS_I2C0_SCLK,
  inout         HPS_I2C0_SDAT,
  inout         HPS_I2C1_SCLK,
  inout         HPS_I2C1_SDAT,

  inout         HPS_KEY,
  inout         HPS_LED,
  inout         HPS_LTC_GPIO,

  output        HPS_SD_CLK,
  inout         HPS_SD_CMD,
  inout   [3:0] HPS_SD_DATA,

  output        HPS_SPIM_CLK,
  input         HPS_SPIM_MISO,
  output        HPS_SPIM_MOSI,
  inout         HPS_SPIM_SS,

  input         HPS_UART_RX,
  output        HPS_UART_TX,

  input         HPS_USB_CLKOUT,
  inout   [7:0] HPS_USB_DATA,
  input         HPS_USB_DIR,
  input         HPS_USB_NXT,
  output        HPS_USB_STP
);



//==============================================================//
// Параметры
//==============================================================//

parameter NK   = 4;                                             // Количество ядер в составе модуля
parameter L2NK = log2(NK);                                      // Логарифм по основанию 2 от NK



//==============================================================//
// Функции
//==============================================================//

//--------------------------------------------------------------//
// Логарифм по основанию 2                                      //

function integer log2;
  input [31:0] value;
  begin
    value = value - 1;

    for( log2 = 0; value > 0; log2 = log2 + 1 )
      value = value >> 1;
  end
endfunction



//==============================================================//
// Внутренние провода/регистры
//==============================================================//

// Провода для соединения с PLL                                 //

wire              pll_clock_main_w;                             // Такты для топ-модуля
// wire            pll_clock_oscill_w;                             // Такты для осциллографа (SignalTap)

// Провода для соединения авалоновского моста с нашим модулем   //

wire       [63:0] mmb_readdata_w;                               // Данные от нас к HPS
wire       [63:0] mmb_writedata_w;                              // Данные от HPS к нам
wire        [6:0] mmb_address_w;                                // Шина адреса
wire              mmb_write_w;                                  // Строб записи
wire              mmb_read_w;                                   // Строб чтения
wire        [7:0] mmb_byteenable_w;                             // Биты разрешения записи

// Исходные данные - записываются процессором HPS               //

reg        [39:0] challenge_reg   = 0;                          // Запрос
reg        [23:0] response_reg    = 0;                          // Ответ
reg        [39:0] start_key_reg   = 0;                          // Стартовый ключ
reg               run_reg         = 0;                          // Разрешение работы ядер

// Результаты поиска                                            //

wire              key_found_w;                                  // Флаг "Ключ найден"
wire              key_not_found_w;                              // Флаг "Работа завершена - ключ не найден"
wire     [NK-1:0] kernels_w;                                    // Биты, показывающие какое ядро (или ядра), нашло ключ
wire  [39-L2NK:0] result_w;                                     // Найденный ключ

reg               irq_reg = 0;                                  // Флаг прерывания



//==============================================================//
// Комбинаторная схемотехника
//==============================================================//

assign GPIO_0 = 36'h ZZZZZZZZZ;
assign GPIO_1 = 36'h ZZZZZZZZZ;

assign LED[7:1] = 7'b 0000000;
assign LED[0]   = key_found_w;



//--------------------------------------------------------------//
// PLL делает такты для всей схемы и для осциллографа           //

pll PLL_INST
(
  .refclk           ( FPGA_CLK1_50        ),                    // Входные такты
  .rst              ( 0                   ),                    // Сброс
  .outclk_0         ( pll_clock_main_w    )                     // Такты для топ-модуля
//  .outclk_1         ( pll_clock_oscill_w  )                     // Такты для осциллографа
);


//--------------------------------------------------------------//
// Модуль поиска ключа, включающий в себя NK хэширующих ядер    //

dst40_XX
#(
  .NK               ( NK   ),
  .L2NK             ( L2NK )
)
DST40_XX_INST
(
  .clock_i          ( pll_clock_main_w        ),                // Такты
  .challenge_i      ( challenge_reg           ),                // Запрос
  .response_i       ( response_reg            ),                // Ответ
  .start_key_i      ( start_key_reg           ),                // Стартовый ключ
  .run_i            ( run_reg                 ),                // Разрешение работы ядер
  .key_found_o      ( key_found_w             ),                // Флаг "ключ найден"
  .key_not_found_o  ( key_not_found_w         ),                // Флаг "ключ не найден"
  .kernels_o        ( kernels_w               ),                // Биты, показывающие какое ядро (или ядра), нашло ключ
  .key_o            ( result_w                )                 // Найденный ключ (младшие биты)
);


//--------------------------------------------------------------//
// HPS-процессор                                                //

soc_system SOC_INST
(
  // Такты и сброс для мостов и прочей периферии
  .clk_clk                                ( FPGA_CLK1_50        ),  // Такты
  .reset_reset_n                          ( 1'b 1               ),  // Сброс

  // RAM DDR3
  .memory_mem_a                           ( HPS_DDR3_ADDR       ),
  .memory_mem_ba                          ( HPS_DDR3_BA         ),
  .memory_mem_ck                          ( HPS_DDR3_CK_P       ),
  .memory_mem_ck_n                        ( HPS_DDR3_CK_N       ),
  .memory_mem_cke                         ( HPS_DDR3_CKE        ),
  .memory_mem_cs_n                        ( HPS_DDR3_CS_N       ),
  .memory_mem_ras_n                       ( HPS_DDR3_RAS_N      ),
  .memory_mem_cas_n                       ( HPS_DDR3_CAS_N      ),
  .memory_mem_we_n                        ( HPS_DDR3_WE_N       ),
  .memory_mem_reset_n                     ( HPS_DDR3_RESET_N    ),
  .memory_mem_dq                          ( HPS_DDR3_DQ         ),
  .memory_mem_dqs                         ( HPS_DDR3_DQS_P      ),
  .memory_mem_dqs_n                       ( HPS_DDR3_DQS_N      ),
  .memory_mem_odt                         ( HPS_DDR3_ODT        ),
  .memory_mem_dm                          ( HPS_DDR3_DM         ),
  .memory_oct_rzqin                       ( HPS_DDR3_RZQ        ),

  // Ethernet
  .hps_0_hps_io_hps_io_emac1_inst_TX_CLK  ( HPS_ENET_GTX_CLK    ),
  .hps_0_hps_io_hps_io_emac1_inst_TXD0    ( HPS_ENET_TX_DATA[0] ),
  .hps_0_hps_io_hps_io_emac1_inst_TXD1    ( HPS_ENET_TX_DATA[1] ),
  .hps_0_hps_io_hps_io_emac1_inst_TXD2    ( HPS_ENET_TX_DATA[2] ),
  .hps_0_hps_io_hps_io_emac1_inst_TXD3    ( HPS_ENET_TX_DATA[3] ),
  .hps_0_hps_io_hps_io_emac1_inst_RXD0    ( HPS_ENET_RX_DATA[0] ),
  .hps_0_hps_io_hps_io_emac1_inst_MDIO    ( HPS_ENET_MDIO       ),
  .hps_0_hps_io_hps_io_emac1_inst_MDC     ( HPS_ENET_MDC        ),
  .hps_0_hps_io_hps_io_emac1_inst_RX_CTL  ( HPS_ENET_RX_DV      ),
  .hps_0_hps_io_hps_io_emac1_inst_TX_CTL  ( HPS_ENET_TX_EN      ),
  .hps_0_hps_io_hps_io_emac1_inst_RX_CLK  ( HPS_ENET_RX_CLK     ),
  .hps_0_hps_io_hps_io_emac1_inst_RXD1    ( HPS_ENET_RX_DATA[1] ),
  .hps_0_hps_io_hps_io_emac1_inst_RXD2    ( HPS_ENET_RX_DATA[2] ),
  .hps_0_hps_io_hps_io_emac1_inst_RXD3    ( HPS_ENET_RX_DATA[3] ),

  // SD-карта
  .hps_0_hps_io_hps_io_sdio_inst_CMD      ( HPS_SD_CMD          ),
  .hps_0_hps_io_hps_io_sdio_inst_D0       ( HPS_SD_DATA[0]      ),
  .hps_0_hps_io_hps_io_sdio_inst_D1       ( HPS_SD_DATA[1]      ),
  .hps_0_hps_io_hps_io_sdio_inst_CLK      ( HPS_SD_CLK          ),
  .hps_0_hps_io_hps_io_sdio_inst_D2       ( HPS_SD_DATA[2]      ),
  .hps_0_hps_io_hps_io_sdio_inst_D3       ( HPS_SD_DATA[3]      ),

  // USB
  .hps_0_hps_io_hps_io_usb1_inst_D0       ( HPS_USB_DATA[0]     ),
  .hps_0_hps_io_hps_io_usb1_inst_D1       ( HPS_USB_DATA[1]     ),
  .hps_0_hps_io_hps_io_usb1_inst_D2       ( HPS_USB_DATA[2]     ),
  .hps_0_hps_io_hps_io_usb1_inst_D3       ( HPS_USB_DATA[3]     ),
  .hps_0_hps_io_hps_io_usb1_inst_D4       ( HPS_USB_DATA[4]     ),
  .hps_0_hps_io_hps_io_usb1_inst_D5       ( HPS_USB_DATA[5]     ),
  .hps_0_hps_io_hps_io_usb1_inst_D6       ( HPS_USB_DATA[6]     ),
  .hps_0_hps_io_hps_io_usb1_inst_D7       ( HPS_USB_DATA[7]     ),
  .hps_0_hps_io_hps_io_usb1_inst_CLK      ( HPS_USB_CLKOUT      ),
  .hps_0_hps_io_hps_io_usb1_inst_STP      ( HPS_USB_STP         ),
  .hps_0_hps_io_hps_io_usb1_inst_DIR      ( HPS_USB_DIR         ),
  .hps_0_hps_io_hps_io_usb1_inst_NXT      ( HPS_USB_NXT         ),

  // SPI 
  .hps_0_hps_io_hps_io_spim1_inst_CLK     ( HPS_SPIM_CLK        ),
  .hps_0_hps_io_hps_io_spim1_inst_MOSI    ( HPS_SPIM_MOSI       ),
  .hps_0_hps_io_hps_io_spim1_inst_MISO    ( HPS_SPIM_MISO       ),
  .hps_0_hps_io_hps_io_spim1_inst_SS0     ( HPS_SPIM_SS         ),

  // UART  
  .hps_0_hps_io_hps_io_uart0_inst_RX      ( HPS_UART_RX         ),
  .hps_0_hps_io_hps_io_uart0_inst_TX      ( HPS_UART_TX         ),

  // I2C1  
  .hps_0_hps_io_hps_io_i2c0_inst_SDA      ( HPS_I2C0_SDAT       ),
  .hps_0_hps_io_hps_io_i2c0_inst_SCL      ( HPS_I2C0_SCLK       ),

  // I2C2  
  .hps_0_hps_io_hps_io_i2c1_inst_SDA      ( HPS_I2C1_SDAT       ),
  .hps_0_hps_io_hps_io_i2c1_inst_SCL      ( HPS_I2C1_SCLK       ),

  // GPIO
  .hps_0_hps_io_hps_io_gpio_inst_GPIO09   ( HPS_CONV_USB_N      ),
  .hps_0_hps_io_hps_io_gpio_inst_GPIO35   ( HPS_ENET_INT_N      ),
  .hps_0_hps_io_hps_io_gpio_inst_GPIO40   ( HPS_LTC_GPIO        ),
  .hps_0_hps_io_hps_io_gpio_inst_GPIO53   ( HPS_LED             ),
  .hps_0_hps_io_hps_io_gpio_inst_GPIO54   ( HPS_KEY             ),
  .hps_0_hps_io_hps_io_gpio_inst_GPIO61   ( HPS_GSENSOR_INT     ),

  // Avalon Memory Mapped Bridge
  .mm_bridge_waitrequest                  ( 0                   ),  // Для записи в регистры нет необходимости в циклах задержки
	.mm_bridge_readdata                     ( mmb_readdata_w      ),  // Передаваемые в HPS данные
	.mm_bridge_readdatavalid                ( mmb_read_w          ),  // При чтении из регистров данные валидны практически сразу
	.mm_bridge_writedata                    ( mmb_writedata_w     ),  // Передаваемые из HPS данные
	.mm_bridge_address                      ( mmb_address_w       ),  // Адрес 64-битного регистра
	.mm_bridge_write                        ( mmb_write_w         ),  // Строб записи
	.mm_bridge_read                         ( mmb_read_w          ),  // Строб чтения
	.mm_bridge_byteenable                   ( mmb_byteenable_w    ),  // Биты разрешения записи

  // IRQ
  .irq0_irq                               ( { 31'd 0, irq_reg } )   // Прерывание IRQ0
);



//--------------------------------------------------------------//
// Чтение данных через интерфейс Avalon-MM                      //
//
// Неиспользуемые старшие биты зануляются, что упрощает
// дальнейшее использование данных в программе.

assign mmb_readdata_w = ( mmb_address_w == 7'd 0 ) ? {           24'b0, challenge_reg                      } :
                        ( mmb_address_w == 7'd 1 ) ? {           40'b0, response_reg                       } :
                        ( mmb_address_w == 7'd 2 ) ? {           24'b0, start_key_reg                      } :
                        ( mmb_address_w == 7'd 3 ) ? {           63'b0, run_reg                            } :
                        ( mmb_address_w == 7'd 4 ) ? {           55'b0, key_not_found_w, 7'b0, key_found_w } :
                        ( mmb_address_w == 7'd 5 ) ? { {L2NK+24{1'b0}}, result_w                           } :
                        ( mmb_address_w == 7'd 6 ) ? {   {64-NK{1'b0}}, kernels_w                          } :
                        0;



//==============================================================//
// Синхронная схемотехника.
//==============================================================//

always @( posedge FPGA_CLK1_50 )
begin

  //------------------------------------------------------------//
  // Запись в регистры через интерфейс Avalon-MM                //
  //
  // Адресация выполняется блоками по 8 байт на 1 адрес,
  // так как ширина интерфейса 64 бита.

  if( mmb_write_w )
  begin

    // Запись в регистр challenge_reg

    if( !mmb_address_w )
    begin
      if( mmb_byteenable_w[0] )
        challenge_reg[7:0]   <= mmb_writedata_w[7:0];

      if( mmb_byteenable_w[1] )
        challenge_reg[15:8]  <= mmb_writedata_w[15:8];

      if( mmb_byteenable_w[2] )
        challenge_reg[23:16] <= mmb_writedata_w[23:16];

      if( mmb_byteenable_w[3] )
        challenge_reg[31:24] <= mmb_writedata_w[31:24];

      if( mmb_byteenable_w[4] )
        challenge_reg[39:32] <= mmb_writedata_w[39:32];
    end

    // Запись в регистр response_reg

    else if( mmb_address_w == 7'd 1 )
    begin
      if( mmb_byteenable_w[0] )
        response_reg[7:0]   <= mmb_writedata_w[7:0];

      if( mmb_byteenable_w[1] )
        response_reg[15:8]  <= mmb_writedata_w[15:8];

      if( mmb_byteenable_w[2] )
        response_reg[23:16] <= mmb_writedata_w[23:16];
    end

    // Запись в регистр start_key_reg

    else if( mmb_address_w == 7'd 2 )
    begin
      if( mmb_byteenable_w[0] )
        start_key_reg[7:0]   <= mmb_writedata_w[7:0];

      if( mmb_byteenable_w[1] )
        start_key_reg[15:8]  <= mmb_writedata_w[15:8];

      if( mmb_byteenable_w[2] )
        start_key_reg[23:16] <= mmb_writedata_w[23:16];

      if( mmb_byteenable_w[3] )
        start_key_reg[31:24] <= mmb_writedata_w[31:24];

      if( mmb_byteenable_w[4] )
        start_key_reg[39:32] <= mmb_writedata_w[39:32];
    end

    // Запись в регистр run_reg

    else if( mmb_address_w == 7'd 3 )
    begin
      if( mmb_byteenable_w[0] )
        run_reg <= mmb_writedata_w[0];
    end
    
    irq_reg <= 0;                                               // По любой записи в любой регистр сбрасываем флаг прерывания

  end
  else
  begin

    if( run_reg && ( key_found_w || key_not_found_w ) )         // Если поиск запущен и взвёлся любой из битов результата поиска,
      irq_reg <= 1;                                             // то взводим флаг прерывания.

  end

end


endmodule
