/******************************************************************************

  Одно ядро DST40 для использования в массиве из XX ядер (больше одного).

******************************************************************************/

module KernelXX
#(
  parameter             NK = 2,                                 // Количество хэширующих ядер в составе модуля
  parameter             L2NK = 1,                               // Логарифм по основанию 2 от количества ядер
  parameter  [L2NK-1:0] ADDRESS = 0                             // Адрес ядра (а фактически - старшие биты ключа)
)
(
  input                 clock_i,                                // Такты
  input                 run_i,                                  // Разрешение работы
  input     [39-L2NK:0] key_i,                                  // Ключ
  input          [39:0] challenge_i,                            // Запрос
  input          [23:0] response_i,                             // Ожидаемый ответ
  output                comparator_o                            // Выход компаратора (1 - результат совпал с ожидаемым ответом)
);




//==============================================================//
// Внутренние провода/регистры
//==============================================================//

wire  [39:0] last_hash_w;



//==============================================================//
// Комбинаторная схемотехника
//==============================================================//

//--------------------------------------------------------------//
// Блок из 64 последовательных модулей Block64                  //

genvar i;

generate

  for( i=0; i < 64; i=i+1 )
  begin: blk64

    wire [39:0] hash_w;
    wire [39:0] key_w;

    if( !i )
    begin
      Block64 BLOCK64_INST
      (
        .clock_i  ( clock_i ),                                  // Такты
        .run_i    ( run_i ),                                    // Разрешение работы
        .hash_i   ( challenge_i ),                              // Входной хэш (такт 0)
        .key_i    ( { ADDRESS, key_i } ),                       // Входной ключ (такт 0)
        .hash_o   ( hash_w ),
        .key_o    ( key_w )
      );
    end
    else if( i < 63 )
    begin
      Block64 BLOCK64_INST
      (
        .clock_i  ( clock_i ),
        .run_i    ( run_i ),
        .hash_i   ( blk64[i-1].hash_w ),
        .key_i    ( blk64[i-1].key_w  ),
        .hash_o   ( hash_w ),
        .key_o    ( key_w )
      );
    end
    else
    begin
      Block64 BLOCK64_INST
      (
        .clock_i  ( clock_i ),
        .run_i    ( run_i ),
        .hash_i   ( blk64[i-1].hash_w ),
        .key_i    ( blk64[i-1].key_w  ),
        .hash_o   ( last_hash_w )
      );
    end
  end
endgenerate


assign comparator_o = ( response_i == last_hash_w[39:16] );


endmodule
