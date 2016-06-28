/*
*		assembler.0.0.2.c is an assembly language assembler program
*		which decodes mneumonics from the file asmtest.mas and
*		outputs a machine code file containing the translated 
*		instructions
*
*		by Shane Kelly
*/

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#define SYMTABSIZE 1000
#define MAX 80
char *label[SYMTABSIZE];
int symbolLine[SYMTABSIZE];
char *opcode[SYMTABSIZE];
char *opnd[SYMTABSIZE];
int lastLine;
short BIGOPCODE = 0;
FILE *fp;
FILE *outFile;
//struct used in printing binary values for char pointers
struct rec
{
	short opcode;
	int bits;
	short dout;
	short aout;
};

// filenames
char inFileName[MAX], outFileName[MAX];

//mneumonics held in a parallel array with opcodes for decoding
char* mneumonics[11] = {
	"ld", "dw", "st", "add", "sub", "jz", "ja", "ldc", "dout", "aout", "halt"
};
short opCodes[11] = {
	0000,
	0000,
	0001,
	2,
	3,
	1100,
	1001,
	8,
	1101,
	1011,
	1111
};

// method used to find the index for a given lable or mneumonic
short getlabel(char *symbolRef, char *array[])
{
	short i = 0;
	
	while(i < lastLine)
	{
		if (strcmp(symbolRef, array[i]) == 0)
			return i;
		i++;
	}
	return -1;
}

// function used to decode opcodes and print them to the machine code file
void decodeAndPrint(char *mneumonic)
{
	struct rec current;	
	current.bits = 0xffff;
	//printf("array index = %d\n", getlabel(mneumonic,mneumonics));
	current.opcode = opCodes[getlabel(mneumonic, mneumonics)];
	current.opcode = (current.opcode << 4) & 0Xf0;
	//printf("current opcode = %d\n", current.opcode);
	if (current.opcode == 112)
		fwrite(&current.bits,sizeof(short), 1, outFile);
	else if (current.opcode == 208)
	{
		current.dout = 0xfffd;
		fwrite(&current.bits, sizeof(char), 1, outFile);
		fwrite(&current.dout, sizeof(char), 1, outFile);
	}
	else if (getlabel(mneumonic,mneumonics) == 9)
	{
		current.aout = 0xfffb;
		fwrite(&current.bits, sizeof(char), 1, outFile);
		fwrite(&current.aout, sizeof(char), 1, outFile);
	}
	else
	{
		fwrite(&current.opcode, sizeof(char), 1, outFile);
		BIGOPCODE = 0;
	}
}

//accessor methods 
char * getopcode(int lineNumber)
{
	return opcode[lineNumber];
}
char * getopnd(int lineNumber)
{
	return opnd[lineNumber];
}

// main function reads in a .mas file and initially constructs a 
// symbol table for each of the labels used in the machine code file

int main(int argc, char *argv[])
{
	char *file_name = argv[2];
	char *token;
	char buffer[100];
	int lineNumber = 0;
	char *input[4096];
	int i = 0;
	int c;
	strcpy(inFileName, argv[1]);
    strcat(inFileName, ".mas");
	fp = fopen(inFileName, "r");
	strcpy(outFileName, argv[1]);
    strcat(outFileName, ".mac");
	outFile = fopen(outFileName, "wb");
	printf("opening assembler file\n");

	// this looping structure reads in the .mas file
	// skipping over whitespaces and comment lines
	// this loop reads the .mas file into a buffer to be used
	// for further access to the machine code file
	while ((c = getc(fp)) != EOF)
	{
    	if (c == ';' || c == ' ' || c == '\n' || c == '\r')
   		 {
    	    // Gobble the rest of the line, or up until EOF
    	    while ((c = getc(fp)) != EOF && c != '\n')
    	        ;
    	}
    	else
    	{
    	    do
    	    {
				ungetc(c, fp);
    	        fgets(buffer, 100, fp);
				input[lineNumber] = strdup(buffer);
				lineNumber++;
    	    } while ((c = getc(fp)) != EOF && c != '\n');
    	}
	}
	lastLine = lineNumber;

	// this looping structure tokenized the input lines
	// from the machine code file into labels, opcodes, and operands
	lineNumber = 0;
	printf("tokenizing input\n");
	while (lineNumber != lastLine)
	{
		label[i] = "";
		if (isspace(input[lineNumber][0]) == 0)
		{
			label[i] = strtok(input[lineNumber], ":");
			opcode[i] = strtok(NULL, " ");
			opnd[i] = strtok(NULL, " ");
			symbolLine[i] = lineNumber;
			i++;
		}
		else
		{
				opcode[i] = strtok(input[lineNumber], " ");
				opnd[i] = strtok(NULL, " ");
				i++;
		}
		lineNumber++;
	}
	lineNumber = 0;
	char *opCode;
	int j = 0;

	//this loop is used to output the machine code file
	printf("outputting .mac file\n");
	while (lineNumber != lastLine)
	{
		//printf("%s\n", getopcode(lineNumber));
		//printf("%s\n", getopnd(lineNumber));
		struct rec current;	
		current.opcode = opCodes[getlabel(getopcode(lineNumber), mneumonics)];
		current.opcode = (current.opcode << 4) & 0Xf0;
		if (current.opcode == 112 || current.opcode == 9 || current.opcode == 208)
		{
			decodeAndPrint(getopcode(lineNumber));
		}
		else if (getlabel(getopnd(lineNumber), label) > 0 && !BIGOPCODE)
		{
			current.opcode = getlabel(getopnd(lineNumber),label);
			//current.opcode = (current.opcode) << 4;
			//printf("printing: %d\n", getlabel(getopnd(lineNumber), label));
			current.opcode--;
			fwrite(&current.opcode, sizeof(char), 1, outFile);
			decodeAndPrint(getopcode(lineNumber));
		}
		else if (strcmp(getopnd(lineNumber), ";") && !BIGOPCODE)
		{
			current.opcode = strtol(getopnd(lineNumber), NULL, 10);
			//printf("printing: %d\n", current.opcode);
			fwrite(&current.opcode, sizeof(char), 1, outFile);
			decodeAndPrint(getopcode(lineNumber));
		}
		//printf("\n");
		lineNumber++;
	}
	fclose(fp);
	fclose(outFile);
	printf("closing outfile\n");
}
