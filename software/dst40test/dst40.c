#include <stdint.h>


typedef union
{
  uint64_t Val;

  struct
  {
    uint8_t b0:1;
    uint8_t b1:1;
    uint8_t b2:1;
    uint8_t b3:1;
    uint8_t b4:1;
    uint8_t b5:1;
    uint8_t b6:1;
    uint8_t b7:1;
    uint8_t b8:1;
    uint8_t b9:1;
    uint8_t b10:1;
    uint8_t b11:1;
    uint8_t b12:1;
    uint8_t b13:1;
    uint8_t b14:1;
    uint8_t b15:1;
    uint8_t b16:1;
    uint8_t b17:1;
    uint8_t b18:1;
    uint8_t b19:1;
    uint8_t b20:1;
    uint8_t b21:1;
    uint8_t b22:1;
    uint8_t b23:1;
    uint8_t b24:1;
    uint8_t b25:1;
    uint8_t b26:1;
    uint8_t b27:1;
    uint8_t b28:1;
    uint8_t b29:1;
    uint8_t b30:1;
    uint8_t b31:1;
    uint8_t b32:1;
    uint8_t b33:1;
    uint8_t b34:1;
    uint8_t b35:1;
    uint8_t b36:1;
    uint8_t b37:1;
    uint8_t b38:1;
    uint8_t b39:1;
    uint8_t reserved1;
    uint8_t reserved2;
    uint8_t reserved3;
  };
} WORD40;



/*
 * Расчёт новых двух старших бит хэша
 */

uint64_t block192( uint64_t hash_in, uint64_t key_in )
{
  uint8_t fa[32] = { 0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1 };
  uint8_t fb[32] = { 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0 };
  uint8_t fc[32] = { 0, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1 };
  uint8_t fd[32] = { 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 1, 0, 0 };
  uint8_t fe[16] = { 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 0, 1, 0 };
  uint8_t fg[16] = { 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0 };
  uint8_t fh[16] = { 0, 0, 2, 3, 3, 1, 2, 1, 1, 2, 1, 3, 3, 2, 0, 0 };

  WORD40 hash;
  WORD40 key;

  hash.Val = hash_in;
  key.Val  = key_in;

  uint8_t fa1  = fa[ (key.b39 << 4) | (key.b31 << 3) | (hash.b39 << 2) | (hash.b31 << 1) | hash.b23 ];
  uint8_t fb2  = fb[ (key.b38 << 4) | (key.b30 << 3) | (hash.b38 << 2) | (hash.b30 << 1) | hash.b22 ];
  uint8_t fc3  = fc[ (key.b23 << 4) | (key.b15 << 3) | (key.b7   << 2) | (hash.b15 << 1) | hash.b7  ];
  uint8_t fd4  = fd[ (key.b22 << 4) | (key.b14 << 3) | (key.b6   << 2) | (hash.b14 << 1) | hash.b6  ];

  uint8_t fa5  = fa[ (key.b37 << 4) | (key.b29 << 3) | (hash.b37 << 2) | (hash.b29 << 1) | hash.b21 ];
  uint8_t fb6  = fb[ (key.b36 << 4) | (key.b28 << 3) | (hash.b36 << 2) | (hash.b28 << 1) | hash.b20 ];
  uint8_t fc7  = fc[ (key.b21 << 4) | (key.b13 << 3) | (key.b5   << 2) | (hash.b13 << 1) | hash.b5  ];
  uint8_t fd8  = fd[ (key.b20 << 4) | (key.b12 << 3) | (key.b4   << 2) | (hash.b12 << 1) | hash.b4  ];

  uint8_t fa9  = fa[ (key.b35 << 4) | (key.b27 << 3) | (hash.b35 << 2) | (hash.b27 << 1) | hash.b19 ];
  uint8_t fb10 = fb[ (key.b34 << 4) | (key.b26 << 3) | (hash.b34 << 2) | (hash.b26 << 1) | hash.b18 ];
  uint8_t fc11 = fc[ (key.b19 << 4) | (key.b11 << 3) | (key.b3   << 2) | (hash.b11 << 1) | hash.b3  ];
  uint8_t fd12 = fd[ (key.b18 << 4) | (key.b10 << 3) | (key.b2   << 2) | (hash.b10 << 1) | hash.b2  ];

  uint8_t fa13 = fa[ (key.b33 << 4) | (key.b25 << 3) | (hash.b33 << 2) | (hash.b25 << 1) | hash.b17 ];
  uint8_t fb14 = fb[ (key.b32 << 4) | (key.b24 << 3) | (hash.b32 << 2) | (hash.b24 << 1) | hash.b16 ];
  uint8_t fe15 = fe[ (key.b17 << 3) | (key.b9  << 2) | (key.b1   << 1) | hash.b9 ];
  uint8_t fe16 = fe[ (key.b16 << 3) | (key.b8  << 2) | (key.b0   << 1) | hash.b8 ];

  uint8_t fg1  = fg[ (fa1  << 3) | (fb2  << 2) | (fc3  << 1) | fd4  ];
  uint8_t fg2  = fg[ (fa5  << 3) | (fb6  << 2) | (fc7  << 1) | fd8  ];
  uint8_t fg3  = fg[ (fa9  << 3) | (fb10 << 2) | (fc11 << 1) | fd12 ];
  uint8_t fg4  = fg[ (fa13 << 3) | (fb14 << 2) | (fe15 << 1) | fe16 ];

  uint8_t fh1  = fh[ (fg1 << 3) | (fg2 << 2) | (fg3 << 1) | fg4 ];

  uint64_t res = fh1 ^ ( (hash.b1 << 1) | hash.b0 );

  return res;
}


/******************************************************************************
 * Функция хэширования 40-битного числа по алгоритму DST40.
 *****************************************************************************/

uint64_t  dst40hash( uint64_t challenge, uint64_t key )
{
  uint8_t i;
  uint8_t cnt;

  uint64_t hash40 = challenge;
  uint64_t key40  = key;

  for( i=0, cnt=0; i < 192; i++ )
  {
    WORD40 tmp;

    hash40 = (block192( hash40, key40 ) << 38) | (hash40 >> 2);

    if( cnt == 1 )
    {
      tmp.Val = key40;

      key40 = ( (uint64_t)(tmp.b0 ^ tmp.b2 ^ tmp.b19 ^ tmp.b21) << 39 ) | (key40 >> 1);
    }

    if( ++cnt == 3 )
      cnt = 0;
  }

  return (hash40 >> 16);
}
