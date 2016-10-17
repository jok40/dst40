/******************************************************************************

  Один из 64 модулей для построения одного комбинаторного ядра DST40.

******************************************************************************/

module Block64
(
  input                 clock_i,                                // Такты
  input                 run_i,                                  // Разрешение работы
  input          [39:0] hash_i,                                 // Входной хэш (такт 0)
  input          [39:0] key_i,                                  // Входной ключ (такт 0)
  output         [39:0] hash_o,                                 // Выходной хэш (такт 3)
  output         [39:0] key_o                                   // Выходной ключ (такт 3)
);


//==============================================================//
// Внутренние провода/регистры
//==============================================================//

wire    [3:0] hi1_w;                                            // Входная шина H на такте 1
wire    [1:0] ho1_w;                                            // Выходная шина H на такте 1

wire    [3:0] hi2_w;                                            // Входная шина H на такте 2
wire    [1:0] ho2_w;                                            // Выходная шина H на такте 2

wire    [3:0] hi3_w;                                            // Входная шина H на такте 3
wire    [1:0] ho3_w;                                            // Выходная шина H на такте 3

wire    [3:0] g1i1_w;                                           // Входная шина G1 на такте 1
wire    [3:0] g2i1_w;                                           // Входная шина G2 на такте 1
wire    [3:0] g3i1_w;                                           // Входная шина G3 на такте 1
wire    [3:0] g4i1_w;                                           // Входная шина G4 на такте 1

wire    [3:0] g1i2_w;                                           // Входная шина G1 на такте 2
wire    [3:0] g2i2_w;                                           // Входная шина G2 на такте 2
wire    [3:0] g3i2_w;                                           // Входная шина G3 на такте 2
wire    [3:0] g4i2_w;                                           // Входная шина G4 на такте 2

wire    [3:0] g1i3_w;                                           // Входная шина G1 на такте 3
wire    [3:0] g2i3_w;                                           // Входная шина G2 на такте 3
wire    [3:0] g3i3_w;                                           // Входная шина G3 на такте 3
wire    [3:0] g4i3_w;                                           // Входная шина G4 на такте 3

wire   [39:0] hash1_w;                                          // HASH после такта 1
wire   [39:0] key1_w;                                           // KEY после такта 1
wire   [39:0] hash2_w;                                          // HASH после такта 2
wire   [39:0] key2_w;                                           // KEY после такта 2
wire   [39:0] hash3_w;                                          // HASH после такта 3
wire   [39:0] key3_w;                                           // KEY после такта 3

reg    [39:0] hash_out_reg;
reg    [39:0] key_out_reg;




//==============================================================//
// Комбинаторная схемотехника
//==============================================================//

assign  hash_o = hash_out_reg;
assign  key_o  = key_out_reg;


//--------------------------------------------------------------//
// Такт 1                                                       //

Fh H1_INST
(
  .in           ( hi1_w ),
  .out          ( ho1_w )
);

Fg G11_INST
(
  .in           ( g1i1_w ),
  .out          ( hi1_w[3] )
);

Fg G21_INST
(
  .in           ( g2i1_w ),
  .out          ( hi1_w[2] )
);

Fg G31_INST
(
  .in           ( g3i1_w ),
  .out          ( hi1_w[1] )
);

Fg G41_INST
(
  .in           ( g4i1_w ),
  .out          ( hi1_w[0] )
);

Fa F11_INST
(
  .in           ( { key_i[39], key_i[31], hash_i[39], hash_i[31], hash_i[23] } ),
  .out          ( g1i1_w[3] )
);

Fb F21_INST
(
  .in           ( { key_i[38], key_i[30], hash_i[38], hash_i[30], hash_i[22] } ),
  .out          ( g1i1_w[2] )
);

Fa F51_INST
(
  .in           ( { key_i[37], key_i[29], hash_i[37], hash_i[29], hash_i[21] } ),
  .out          ( g2i1_w[3] )
);

Fb F61_INST
(
  .in           ( { key_i[36], key_i[28], hash_i[36], hash_i[28], hash_i[20] } ),
  .out          ( g2i1_w[2] )
);

Fa F91_INST
(
  .in           ( { key_i[35], key_i[27], hash_i[35], hash_i[27], hash_i[19] } ),
  .out          ( g3i1_w[3] )
);

Fb F101_INST
(
  .in           ( { key_i[34], key_i[26], hash_i[34], hash_i[26], hash_i[18] } ),
  .out          ( g3i1_w[2] )
);

Fa F131_INST
(
  .in           ( { key_i[33], key_i[25], hash_i[33], hash_i[25], hash_i[17] } ),
  .out          ( g4i1_w[3] )
);

Fb F141_INST
(
  .in           ( { key_i[32], key_i[24], hash_i[32], hash_i[24], hash_i[16] } ),
  .out          ( g4i1_w[2] )
);

Fc F31_INST
(
  .in           ( { key_i[23], key_i[15], key_i[7], hash_i[15], hash_i[7] } ),
  .out          ( g1i1_w[1] )
);

Fd F41_INST
(
  .in           ( { key_i[22], key_i[14], key_i[6], hash_i[14], hash_i[6] } ),
  .out          ( g1i1_w[0] )
);

Fc F71_INST
(
  .in           ( { key_i[21], key_i[13], key_i[5], hash_i[13], hash_i[5] } ),
  .out          ( g2i1_w[1] )
);

Fd F81_INST
(
  .in           ( { key_i[20], key_i[12], key_i[4], hash_i[12], hash_i[4] } ),
  .out          ( g2i1_w[0] )
);

Fc F111_INST
(
  .in           ( { key_i[19], key_i[11], key_i[3], hash_i[11], hash_i[3] } ),
  .out          ( g3i1_w[1] )
);

Fd F121_INST
(
  .in           ( { key_i[18], key_i[10], key_i[2], hash_i[10], hash_i[2] } ),
  .out          ( g3i1_w[0] )
);

Fe F151_INST
(
  .in           ( { key_i[17], key_i[9], key_i[1], hash_i[9] } ),
  .out          ( g4i1_w[1] )
);

Fe F161_INST
(
  .in           ( { key_i[16], key_i[8], key_i[0], hash_i[8] } ),
  .out          ( g4i1_w[0] )
);


//--------------------------------------------------------------//
// Результат такта 1 / Исходные данные для такта 2              //

assign hash1_w = { ho1_w ^ hash_i[1:0], hash_i[39:2] };
assign key1_w  = key_i;


//--------------------------------------------------------------//
// Такт 2                                                       //

Fh H2_INST
(
  .in           ( hi2_w ),
  .out          ( ho2_w )
);

Fg G12_INST
(
  .in           ( g1i2_w ),
  .out          ( hi2_w[3] )
);

Fg G22_INST
(
  .in           ( g2i2_w ),
  .out          ( hi2_w[2] )
);

Fg G32_INST
(
  .in           ( g3i2_w ),
  .out          ( hi2_w[1] )
);

Fg G42_INST
(
  .in           ( g4i2_w ),
  .out          ( hi2_w[0] )
);

Fa F12_INST
(
  .in           ( { key1_w[39], key1_w[31], hash1_w[39], hash1_w[31], hash1_w[23] } ),
  .out          ( g1i2_w[3] )
);

Fb F22_INST
(
  .in           ( { key1_w[38], key1_w[30], hash1_w[38], hash1_w[30], hash1_w[22] } ),
  .out          ( g1i2_w[2] )
);

Fa F52_INST
(
  .in           ( { key1_w[37], key1_w[29], hash1_w[37], hash1_w[29], hash1_w[21] } ),
  .out          ( g2i2_w[3] )
);

Fb F62_INST
(
  .in           ( { key1_w[36], key1_w[28], hash1_w[36], hash1_w[28], hash1_w[20] } ),
  .out          ( g2i2_w[2] )
);

Fa F92_INST
(
  .in           ( { key1_w[35], key1_w[27], hash1_w[35], hash1_w[27], hash1_w[19] } ),
  .out          ( g3i2_w[3] )
);

Fb F102_INST
(
  .in           ( { key1_w[34], key1_w[26], hash1_w[34], hash1_w[26], hash1_w[18] } ),
  .out          ( g3i2_w[2] )
);

Fa F132_INST
(
  .in           ( { key1_w[33], key1_w[25], hash1_w[33], hash1_w[25], hash1_w[17] } ),
  .out          ( g4i2_w[3] )
);

Fb F142_INST
(
  .in           ( { key1_w[32], key1_w[24], hash1_w[32], hash1_w[24], hash1_w[16] } ),
  .out          ( g4i2_w[2] )
);

Fc F32_INST
(
  .in           ( { key1_w[23], key1_w[15], key1_w[7], hash1_w[15], hash1_w[7] } ),
  .out          ( g1i2_w[1] )
);

Fd F42_INST
(
  .in           ( { key1_w[22], key1_w[14], key1_w[6], hash1_w[14], hash1_w[6] } ),
  .out          ( g1i2_w[0] )
);

Fc F72_INST
(
  .in           ( { key1_w[21], key1_w[13], key1_w[5], hash1_w[13], hash1_w[5] } ),
  .out          ( g2i2_w[1] )
);

Fd F82_INST
(
  .in           ( { key1_w[20], key1_w[12], key1_w[4], hash1_w[12], hash1_w[4] } ),
  .out          ( g2i2_w[0] )
);

Fc F112_INST
(
  .in           ( { key1_w[19], key1_w[11], key1_w[3], hash1_w[11], hash1_w[3] } ),
  .out          ( g3i2_w[1] )
);

Fd F122_INST
(
  .in           ( { key1_w[18], key1_w[10], key1_w[2], hash1_w[10], hash1_w[2] } ),
  .out          ( g3i2_w[0] )
);

Fe F152_INST
(
  .in           ( { key1_w[17], key1_w[9], key1_w[1], hash1_w[9] } ),
  .out          ( g4i2_w[1] )
);

Fe F162_INST
(
  .in           ( { key1_w[16], key1_w[8], key1_w[0], hash1_w[8] } ),
  .out          ( g4i2_w[0] )
);


//--------------------------------------------------------------//
// Результат такта 2 / Исходные данные для такта 3              //

assign hash2_w = { ho2_w ^ hash1_w[1:0], hash1_w[39:2] };
assign key2_w  = { key1_w[0] ^ key1_w[2] ^ key1_w[19] ^ key1_w[21], key1_w[39:1] };


//--------------------------------------------------------------//
// Такт 3                                                       //

Fh H3_INST
(
  .in           ( hi3_w ),
  .out          ( ho3_w )
);

Fg G13_INST
(
  .in           ( g1i3_w ),
  .out          ( hi3_w[3] )
);

Fg G23_INST
(
  .in           ( g2i3_w ),
  .out          ( hi3_w[2] )
);

Fg G33_INST
(
  .in           ( g3i3_w ),
  .out          ( hi3_w[1] )
);

Fg G43_INST
(
  .in           ( g4i3_w ),
  .out          ( hi3_w[0] )
);

Fa F13_INST
(
  .in           ( { key2_w[39], key2_w[31], hash2_w[39], hash2_w[31], hash2_w[23] } ),
  .out          ( g1i3_w[3] )
);

Fb F23_INST
(
  .in           ( { key2_w[38], key2_w[30], hash2_w[38], hash2_w[30], hash2_w[22] } ),
  .out          ( g1i3_w[2] )
);

Fa F53_INST
(
  .in           ( { key2_w[37], key2_w[29], hash2_w[37], hash2_w[29], hash2_w[21] } ),
  .out          ( g2i3_w[3] )
);

Fb F63_INST
(
  .in           ( { key2_w[36], key2_w[28], hash2_w[36], hash2_w[28], hash2_w[20] } ),
  .out          ( g2i3_w[2] )
);

Fa F93_INST
(
  .in           ( { key2_w[35], key2_w[27], hash2_w[35], hash2_w[27], hash2_w[19] } ),
  .out          ( g3i3_w[3] )
);

Fb F103_INST
(
  .in           ( { key2_w[34], key2_w[26], hash2_w[34], hash2_w[26], hash2_w[18] } ),
  .out          ( g3i3_w[2] )
);

Fa F133_INST
(
  .in           ( { key2_w[33], key2_w[25], hash2_w[33], hash2_w[25], hash2_w[17] } ),
  .out          ( g4i3_w[3] )
);

Fb F143_INST
(
  .in           ( { key2_w[32], key2_w[24], hash2_w[32], hash2_w[24], hash2_w[16] } ),
  .out          ( g4i3_w[2] )
);

Fc F33_INST
(
  .in           ( { key2_w[23], key2_w[15], key2_w[7], hash2_w[15], hash2_w[7] } ),
  .out          ( g1i3_w[1] )
);

Fd F43_INST
(
  .in           ( { key2_w[22], key2_w[14], key2_w[6], hash2_w[14], hash2_w[6] } ),
  .out          ( g1i3_w[0] )
);

Fc F73_INST
(
  .in           ( { key2_w[21], key2_w[13], key2_w[5], hash2_w[13], hash2_w[5] } ),
  .out          ( g2i3_w[1] )
);

Fd F83_INST
(
  .in           ( { key2_w[20], key2_w[12], key2_w[4], hash2_w[12], hash2_w[4] } ),
  .out          ( g2i3_w[0] )
);

Fc F113_INST
(
  .in           ( { key2_w[19], key2_w[11], key2_w[3], hash2_w[11], hash2_w[3] } ),
  .out          ( g3i3_w[1] )
);

Fd F123_INST
(
  .in           ( { key2_w[18], key2_w[10], key2_w[2], hash2_w[10], hash2_w[2] } ),
  .out          ( g3i3_w[0] )
);

Fe F153_INST
(
  .in           ( { key2_w[17], key2_w[9], key2_w[1], hash2_w[9] } ),
  .out          ( g4i3_w[1] )
);

Fe F163_INST
(
  .in           ( { key2_w[16], key2_w[8], key2_w[0], hash2_w[8] } ),
  .out          ( g4i3_w[0] )
);


//--------------------------------------------------------------//
// Результат такта 3                                            //

assign hash3_w = { ho3_w ^ hash2_w[1:0], hash2_w[39:2] };
assign key3_w  = key2_w;



//==============================================================//
// Синхронная схемотехника.
//==============================================================//

//--------------------------------------------------------------//
// Защёлкивание результата в регистры.

always @( posedge clock_i )
begin

  if( run_i )
  begin
    hash_out_reg <= hash3_w;
    key_out_reg  <= key3_w;
  end

end


endmodule
