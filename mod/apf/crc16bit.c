#ifndef CRC16BIT_C
#define CRC16BIT_C

#include <crc16bit.h>

/* initialize CRC with `1111 1111 1111 1111' (less the msb) */
static const unsigned int CrcInit=0xffff;

/* initialize kernel with `1 0001 0000 0010 0001' (less the msb) */
static const unsigned int kernel=0x1021;

/*========================================================================*/
/* 16-bit CRC generator based on primitive polynomials modulo 2           */
/*========================================================================*/
/**
   This function generates an 16-bit CRC for a byte array of arbitrary
   length.  It should be completely portable to differing architectures as
   long as the type `unsigned char' is exactly 8 bits in length...which
   covers any c-implementation on any architecture & operating system that I
   am aware of.  Also required is that `unsigned int' be 16 bits or
   more. Unfortunately, the ANSI C-Standard merely stipulates that `unsigned
   char' be a *minimum* of 8-bits...it could have more and still be ANSI
   compliant.  However, this function will break if `unsigned char' does not
   have exactly 8 bits.  The demands of portability were given preference to
   those of efficiency.  More efficient implementations are possible but
   portability across both big-endian and little-endian architectures would
   be sacrificed.

   This is only a 16-bit CRC so if you want to use the CRC as a mapping hash
   (ie., the index for an associative array) don't get carried away with the
   message length.  If you consider informational entropy for just a minute
   you will see that the number of distinct N-bit messages that are assigned
   the same CRC will approach (on average) 2^(N-16).  Clearly, the larger the
   pool of messages that have the same CRC, the higher the probability that
   two messages will encounter a hashing collision.

   The algorithm used exhibits one property that complements message
   transmission characteristics.  Corruptions tend to come in bursts with
   packets of adjacent bits scrambled.  Regardless of the message length, if
   the corrupt bits occur only within some window 16 bits (or fewer) wide,
   then this CRC is guaranteed to detect the error.  Of course, 16 bits might
   be a bit narrow for that guarantee to be very soothing.

   The best reference that I know of for an introduction to this topic is
   the amazing book `Numerical Recipes in C: The Art of Scientific
   Computing, Second Edition' by Press, Teukolsky, Vetterling, & Flannery
   (ISBN 0-521-43108-5).  While the implementation is strictly my own, much
   of what is written within this comment section is taken or paraphrased
   from that source.
   
   The mathematical theory on which this algorithm is based is that of
   "polynomials over the integers modulo 2".  Any binary string can be
   considered to encode a polynomial whose coefficients are either 0 or 1.
   For example, the bit string "1010101010101010" is shorthand for the
   polynomial x^15 + x^13 + x^11 + x^9 + x^7 + x^5 + x^3 + x^1.  Since 0 and
   1 are the only integers modulo 2, a power of x in the polynomial is
   either present (1) or absent (0).  Any such polynomial has a unique
   factorization into irreducible primitive polynomials.  The polynomial
   x^2 + x + 1 is primitive, while x^2 + 1 is not: x^2 + 1 = (x+1)(x+1)
   [modulo 2].  An M-bit CRC is based on a particular primitive polynomial
   of degree M, called the generator polynomial, G.

   Given an arbitrary polynomial S (encoded as a bit string) and the
   generator polynomial G of degree M, there are three steps to computing
   the CRC:

      1. Multiply S by x^M. (Bit-shift left by M bits.)

      2. Divide (by long division) the polynomial (Sx^M) by the primitive
      polynomial G.  Note that modulo-2 subtraction is the same as logical
      exclusive-or (XOR).  The bit manipulations involved are merely
      left-shifting and XOR's at the right places.

      3. The remainder after the long division is done is the CRC.  The
      quotient is totally ignored.  The remainder is guaranteed to be of
      order M-1 or less.  Therefore, in bit string form, it has M bits which
      may include leading zeros.

   As described above, the algorithm is not capable of detecting any errors
   in the leading segment of the message that contains only zeros.  This
   defect is corrected if the bit string, S, is appended to an M-bit string
   consisting of all 1's.  This variation will be labeled step 0.
   
   For example, consider the 15th order polynomial S = "1010101010101010"
   and the 16th order primitive generator polynomial G="10001000000100001".
   Here is how to generate the 16-bit CRC for the message string S:

      0. Append S to the 16-bit string "111111111111111" to form S'.
           S' =  "1111111111111111010101010101010" 
   
      1. Multiply S' by x^16 to yeild:
           S'x^16 = "111111111111111110101010101010100000000000000000"
         
      2. Synthetic division of S'x^16 by G is now done (modulo 2):
   
                        1111000011101111101111000111010 remainder 0110001011010101
                      +----------------------------------------------------------
    10001000000100001 | 111111111111111110101010101010100000000000000000
                        10001000000100001 (xor)
                        -----------------
                        011101111110111100
                         10001000000100001 (xor)
                         -----------------
                         011001111100111011
                          10001000000100001 (xor)
                          -----------------
                          010001111000110100
                           10001000000100001 (xor)
                           -----------------
                           0000011100001010110101
                                10001000000100001 (xor)
                                -----------------
                                011010010100101000
                                 10001000000100001 (xor)
                                 -----------------
                                 010110101000010011
                                  10001000000100001 (xor)
                                  -----------------
                                  00111101000110010
                                    10001000000100001 (xor)
                                    -----------------
                                    011111000111010000
                                     10001000000100001 (xor)
                                     -----------------
                                     01110000111110001
                                      10001000000100001 (xor)
                                      -----------------
                                      011010011110000100
                                       10001000000100001 (xor)
                                       -----------------
                                       010110111101001010
                                        10001000000100001 (xor)
                                        -----------------
                                        0011111110110101100
                                          10001000000100001 (xor)
                                          -----------------
                                          011101101100011010
                                           10001000000100001 (xor)
                                           -----------------
                                           011001011001110110
                                            10001000000100001 (xor)
                                            -----------------
                                            010000110010101110
                                             10001000000100001 (xor)
                                             -----------------
                                             00001110010001111
                                                 10001000000100001 (xor)
                                                 -----------------
                                                 011011000110100010
                                                  10001000000100001 (xor)
                                                  -----------------
                                                  010100001100000110
                                                   10001000000100001 (xor)
                                                   -----------------
                                                   0010100110010011100
                                                     10001000000100001 (xor)
                                                     -----------------
                                                     0010111001011110100
                                                       10001000000100001 (xor)
                                                       -----------------
                                                  CRC = 0110001011010101

      3. The remainder (0x62d5) is the CRC.  The quotient is simply ignored.

   That's all there is to it...pretty simple really.  If you work through
   this example, you will find that what you are really doing is
   left-shifting sequential bits of S, from the right, into an M-bit
   register.  Every time a 1 gets shifted off the left end of the register,
   you zap the register by an XOR with the M low order bits of G (ie., all
   the bits of G except for its leading 1).  When a 0 is left-shifted out of
   the register you don't zap the register.  When the last bit that was
   originally part of S get shifted of the left-end of the register then
   what remains is the CRC.

   This process is a natural for hardware.  And easily accomplished in
   software as well.  As you can see below, it can be done (albiet with some
   inefficiency) with a couple of loops and bits of gymnastics.  The 16th  
   order primitive polynomial "10001000000100001" is referred to as the
   "CCITT polynomial".  It is the same primitive polynomial used by many   
   commercial protocols (due to algorithmic variations, this implementation
   will not yield the same CRC as commercial protocols).  See section 20.3,
   p896-901 of Numerical Recipes in C for more details (reference previously
   cited).

   This function has been tested to make sure I haven't fallen into any
   idiot traps.  The tests were statistical in nature.  I wanted to verify
   that random 32 byte messages were distributed uniformly amongst 2^16
   (65536) bins.  I generated 655.36 million random 32 byte messages and
   applied the 2-byte CRC generator to each.  The acid test is a uniform
   probability distribution for the CRC (over a random ensemble of
   messages).  Because there are 65536 degrees of freedom for the CRC
   generator, the number of random messages that have to be generated in
   order to ensure stable statistics becomes rather large.  655.36 million
   maybe a bit over the top, but what the heck...computer time is free these
   days, and I'm on a long cruise.  Anyway, 655.36 million random 32 byte
   messages is enough to exercise each degree of freedom an average of
   10,000 times.  Hence, the resulting statistics are very stable.  The
   standard deviation around this mean should theoretically go as its square
   root and therefore is expected to be 100. The actual frequency
   distribution is shown on p146, '97 SWJ.  The actual mean and standard
   deviation were computed to be 9994.5 and 99.8, resp.  Hence, this CRC
   generator is behaving virtually as expected.

      Check values:
         +------------------------------------------------------+
         | message                            | length |    CRC |
         +------------------------------------+--------+--------+
         | -none-                             |      0 | 0x1d0f |
         | "A"                                |      1 | 0x9479 |
         | "123456789"                        |      9 | 0xe5cc |
         | "SwiftWare::Crc16Bit()"            |     21 | 0xf293 |
         | "AAAAA...AAAA" (string of 256 A's) |    256 | 0xe938 |
         +------------------------------------------------------+

   Written by Dana Swift
*/
unsigned int Crc16Bit(const unsigned char *msg,unsigned int len)
{
   unsigned int byte,i;
   
   /* initialize the crc with all 1's */
   unsigned int crc=CrcInit;

   /* eat the message, one byte at a time */
   for (byte=0; byte<len+2; byte++)
   {
      /* mask to locate the next bit to shift into the register's right side */
      unsigned char mask = 0x80;

      for (i=0; i<8; i++)
      {
         /* record if a high bit is about to be shifted out of the register */
         int xor = (crc&0x8000);

         /* left shift the register by 1 bit */
         crc <<= 1;

         /* shift the next message bit into the right side of the register */
         if (byte<len && mask&msg[byte]) {crc |= 0x0001;} mask>>=1;

         /* divide (modulo 2) the current polynomial by the prime polynomial */
         if (xor) crc = ((crc&0xffff) ^ kernel);
      }
   }
   
   return crc;
}

/*------------------------------------------------------------------------*/
/* function to initialize the CRC calculation                             */
/*------------------------------------------------------------------------*/
unsigned int Crc16BitInit(unsigned int *crc)
{
   /* initialize the crc with all 1's */
   (*crc)=CrcInit;

   return (*crc);
}

/*------------------------------------------------------------------------*/
/* function to iterate the CRC calculation using a message byte           */
/*------------------------------------------------------------------------*/
unsigned int Crc16BitIterate(unsigned int *crc, unsigned char byte)
{
   /* mask to locate the next bit to shift into the register's right side */
   int i; unsigned char mask = 0x80;

   for (i=0; i<8; i++)
   {
      /* record if a high bit is about to be shifted out of the register */
      int xor = ((*crc)&0x8000);

      /* left shift the register by 1 bit */
      (*crc) <<= 1;

      /* shift the next message bit into the right side of the register */
      if (mask&byte) {(*crc) |= 0x0001;} mask>>=1;

      /* divide (modulo 2) the current polynomial by the prime polynomial */
      if (xor) (*crc) = (((*crc)&0xffff) ^ kernel);
   }
   
   return (*crc);
}

/*------------------------------------------------------------------------*/
/* function to terminate the CRC calculation                              */
/*------------------------------------------------------------------------*/
unsigned int Crc16BitTerminate(unsigned int *crc)
{
   unsigned int byte,i;

   /* eat the message, one byte at a time */
   for (byte=0; byte<2; byte++)
   {
      for (i=0; i<8; i++)
      {
         /* record if a high bit is about to be shifted out of the register */
         int xor = ((*crc)&0x8000);

         /* left shift the register by 1 bit */
         (*crc) <<= 1;

         /* divide (modulo 2) the current polynomial by the prime polynomial */
         if (xor) (*crc) = (((*crc)&0xffff) ^ kernel);
      }
   }
   
   return (*crc);
}

#endif /* CRC16BIT_C */
