#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// insperation from https://github.com/Ewpratten/binmap
// its the same but not just green. and it is c.

void insertLongInArray(uint8_t *array, int startIndex, uint32_t number) {
    uint8_t twoFityFive = 255;
    uint8_t bytes[4]; 
    
    bytes[0] = ((uint8_t)(number >> 24)) & twoFityFive;
    bytes[1] = ((uint8_t)(number >> 16)) & twoFityFive;
    bytes[2] = ((uint8_t)(number >> 8)) & twoFityFive;
    bytes[3] = ((uint8_t)number) & twoFityFive;

    // printf("size: %ld ",number);
    // printf("result: %ld %ld %ld %ld \n",bytes[0],bytes[1],bytes[2],bytes[3]);

    if(bytes[0] == 0 && bytes[1] == 0 && bytes[2] == 0) {
        array[startIndex] = bytes[3];
    } else if(bytes[0] == 0 && bytes[1] == 0) {
        array[startIndex] = bytes[2];
        array[startIndex+1] = bytes[3];
    } else if(bytes[0] == 0) {
        array[startIndex] = bytes[1];
        array[startIndex+1] = bytes[2];
        array[startIndex+2] = bytes[3];
    } else {
        array[startIndex] = bytes[0];
        array[startIndex+1] = bytes[1];
        array[startIndex+2] = bytes[2];
        array[startIndex+3] = bytes[3];
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("missing file name: file2img.exe [file]");
        return 22;
    }
    printf("opening: %s\n", argv[1]);

    // open specified file
    FILE *filePointer = fopen(argv[1], "rb"); // WHY WINDOWS! SRSLY YOU NEED rb INSTEAD OF r SO YOU WON'T RANDOMLY STOP READING IF YOU ENCOUNTER A 0x1a?!?! bruh
    if (!filePointer) {
        perror("cannot open file");
        return 2;
    }

    // get file length
    fseek(filePointer, 0, SEEK_END);
    long sideLength = ftell(filePointer);
    fseek(filePointer, 0, SEEK_SET);

    // find smallest square possible for file
    sideLength = ceil(sqrt((double)ceill(sideLength)) / 3) * 3;
    long fileSize = 54 + sideLength * sideLength;

    // create string for image data
    uint8_t *image = malloc(fileSize);
    if (image == NULL) {
        perror("malloc failed");
        exit(1);
    }

    memset(image, 0, fileSize);

    // -- FILE HEADER -- https://stackoverflow.com/questions/11004868/creating-a-bmp-file-bitmap-in-c

    // bitmap signature
    image[0] = 'B';
    image[1] = 'M';

    // file size
    printf("file size: %ld side length: %ld\n",fileSize,sideLength);
    insertLongInArray(image, 2, (uint32_t)fileSize);

    // reserved field (in hex. 00 00 00 00)
    for (int i = 6; i < 10; i++) image[i] = 0;

    // offset of pixel data inside the image
    image[10] = 54;

    // header size
    image[14] = 40;

    // width of the image
    insertLongInArray(image, 18, sideLength/3);

    // height of the image
    insertLongInArray(image, 22, sideLength/3);

    // reserved field
    image[26] = 1;

    // number of bits per pixel
    image[28] = 24;  // 3 byte


    // size of pixel data
    insertLongInArray(image, 34, sideLength * sideLength);

    // write pixel data to header
    int c;
    int i = 54;
    while ((c = fgetc(filePointer)) != EOF) {
        image[i] = c;
        i++;
    }

    // create image
    FILE *imgOut;
    imgOut = fopen("output.bmp", "w");
    if (!imgOut) {
        perror("cannot make image");
        return 2;
    }

    // write data to image
    for (int j = 0; j < fileSize; j++) {
        fputc(image[j], imgOut);
    }

    free(image);
    fclose(filePointer);
    fclose(imgOut);

    printf("image created (hopefully)");
    return 0;
}