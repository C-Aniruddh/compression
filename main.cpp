/*
 * Middle Out Compression Algorithm
 *
 * File created by Aniruddh Chandratre on 28 June 2017.
 *
*/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#define bit_one 11
#define bit_two 4
#define PARA 1
#define L (1 << bit_one)
#define L1 ((1 << bit_two) + 1)

int bit_buffer = 0, bit_mask = 128;
unsigned long input_value = 0;
unsigned long output_value = 0;
unsigned char buffer[L * 2];
FILE *infile, *outfile;


class Compression{
public:
    void error();
    void putbit(int value);
    int getbit(int n);
    void flush_buffer();
    void output(int c, int x, int y, int value);
    void compress();
    void uncompress();
};

void Compression::error(){
    std::cout << "There was an error processing your file. Please try again." << std::endl;
    exit(1);
}

void Compression::putbit(int value) {
    if (value == 1){
        bit_buffer |= bit_mask;

        if((bit_mask >>= 1) == 0) {
            if(fputc(bit_buffer, outfile) == EOF) {
                error();
            }
            bit_buffer = 0;
            bit_mask = 128;

            output_value++;
        }
    } else if (value == 0){
        if((bit_mask >>= 1) == 0) {
            if(fputc(bit_buffer, outfile) == EOF) {
                error();
            }

            bit_buffer = 0;
            bit_mask = 128;

            output_value++;
        }
    }
}

void Compression::flush_buffer() {
    if(bit_mask != 128) {
        if(fputc(bit_buffer, outfile) == EOF) {
            error();
        }
        output_value++;
    }
}

void Compression::output(int c, int x, int y,int value) {
    int mask;

    if (value == 1){
        putbit(1);
        mask = 256;
        while(mask >>= 1) {
            if(c & mask) {
                putbit(1);
            } else {
                putbit(0);
            }
        }
    } else if (value == 2){
        putbit(0);
        mask = L;
        while(mask >>= 1) {
            if(x & mask) {
                putbit(1);
            } else {
                putbit(0);
            }
        }

        mask = (1 << bit_two);
        while(mask >>= 1) {
            if(y & mask) {
                putbit(1);
            } else {
                putbit(0);
            }
        }
    } else {
        std::cout << "Value error in output. " << std::endl;
    }
}

void Compression::compress()
{
    int i, j, f1, x, y, r, s, bufferend, c;
    for(i = 0; i < (L - L1); i++) {
        buffer[i] = ' ';
    }

    for(i = (L - L1); i < (L * 2); i++) {
        if((c = fgetc(infile)) == EOF) {
            break;
        }
        buffer[i] = (unsigned char) c;
        input_value++;
    }

    bufferend = i;
    r = (L - L1);
    s = 0;

    while(r < bufferend) {
        f1 = (L <= (bufferend - r)) ? L : (bufferend - r);
        x = 0;
        y = 1;
        c = buffer[r];

        for(i = (r - 1); i >= s; i--) {
            if(buffer[i] == c) {
                for(j = 1; j < f1; j++) {
                    if(buffer[i + j] != buffer[r + j]) {
                        break;
                    }
                }
                if(j > y) {
                    x = i;
                    y = j;
                }
            }
        }

        if(y <= PARA) {
            y = 1;
            output(c, 0, 0, 1);
        } else {
            int pass_x = x & (L - 1);
            int pass_y = y - 2;
            output(0, pass_x, pass_y, 2);
        }

        r += y;
        s += y;

        if(r >= (L * 2 - L1)) {
            for(i = 0; i < L; i++) {
                buffer[i] = buffer[i + L];
            }

            bufferend -= L;
            r -= L;
            s -= L;

            while(bufferend < (L * 2)) {
                if((c = fgetc(infile)) == EOF) {
                    break;
                }
                buffer[bufferend++] = (unsigned char) c;
                input_value++;
            }
        }
    }
    flush_buffer();
    std::cout << "Input size = " << input_value << "bytes" << std::endl;
    std::cout << "Output size = " << output_value << "bytes" << std::endl;

}

int Compression::getbit(int n) {
    int i, x = 0;
    static int buf, mask = 0;

    for(i = 0; i < n; i++) {
        if(mask == 0) {
            if((buf = fgetc(infile)) == EOF) {
                return EOF;
            }
            mask = 128;
        }
        x <<= 1;
        if(buf & mask) {
            x++;
        }
        mask >>= 1;
    }
    return x;
}

void Compression::uncompress() {
    int i, j, k, r, c;
    for(i = 0; i < (L - L1); i++) {
        buffer[i] = ' ';
    }
    r = (L - L1);
    while((c = getbit(1)) != EOF) {
        if(c) {
            if((c = getbit(8)) == EOF) {
                break;
            }
            fputc(c, outfile);
            buffer[r++] = (unsigned char) c;
            r &= (L - 1);
        } else {
            if((i = getbit(bit_one)) == EOF) {
                break;
            }
            if((j = getbit(bit_two)) == EOF) {
                break;
            }
            for(k = 0; k <= (j + 1); k++) {
                c = buffer[(i + k) & (L - 1)];
                fputc(c, outfile);
                buffer[r++] = (unsigned char) c;
                r &= (L - 1);
            }
        }
    }
}

int main(int argc, char *argv[])
{
    int compression;
    char *instruction;

    Compression comp;

    if(argc != 4) {
        std::cout << "Refer to --help for usage instructions." << std::endl;
        return 1;
    }

    instruction = argv[1];

    if(*instruction == 'x' || (*instruction == 'c')) {
        compression = *instruction == 'c';
    } else {
        std::cout << instruction << std::endl;
        return 1;
    }

    if((infile  = fopen(argv[2], "rb")) == NULL) {
        std::cout << "? " << argv[2] << std::endl;
        return 1;
    }

    if((outfile = fopen(argv[3], "wb")) == NULL) {
        std::cout << "? " << argv[3] << std::endl;
        return 1;
    }

    if(compression) {
        comp.compress();
    } else {
        comp.uncompress();
    }
    fclose(infile);
    fclose(outfile);

    return 0;
}