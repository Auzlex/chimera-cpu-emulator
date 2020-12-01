/*

	Chimera-2018-J,
	Assembly Language Processor Emulator,
	CANS Technology, Inc.

	Project Started: 20/01/2020

	Scripted By Charles Edwards.
	Last Revision Date: 28/02/2020

	User Help:
	To run this emulator please use the markingserver.exe that is provided with the project.

*/

#include "stdafx.h"
#include <winsock2.h>
#include <string>

#pragma comment(lib, "wsock32.lib")
#define STUDENT_NUMBER    "19008097"
#define IP_ADDRESS_SERVER "127.0.0.1"
#define PORT_SERVER 0x1984 // We define a port that we are going to use.
#define PORT_CLIENT 0x1985 // We define a port that we are going to use.
#define WORD  unsigned short
#define DWORD unsigned long
#define BYTE  unsigned char
#define MAX_FILENAME_SIZE 500
#define MAX_BUFFER_SIZE   500

SOCKADDR_IN server_addr;
SOCKADDR_IN client_addr;

SOCKET sock;  // This is our socket, it is the handle to the IO address to read/write packets

WSADATA data;

char InputBuffer [MAX_BUFFER_SIZE];

char hex_file [MAX_BUFFER_SIZE];
char trc_file [MAX_BUFFER_SIZE];

//////////////////////////
//   Registers          //
//////////////////////////

#define FLAG_I  0x80
#define FLAG_N  0x20
#define FLAG_V  0x10
#define FLAG_Z  0x08
#define FLAG_C  0x01
#define REGISTER_A	6
#define REGISTER_H	5
#define REGISTER_L	4
#define REGISTER_E	3
#define REGISTER_D	2
#define REGISTER_C	1
#define REGISTER_B	0

// added registers
#define REGISTER_M 7//0//7//4
WORD IndexRegister;

BYTE Registers[8]; // was 7
BYTE Flags;
WORD ProgramCounter;
WORD StackPointer;


////////////
// Memory //
////////////

#define MEMORY_SIZE	65536

BYTE Memory[MEMORY_SIZE];

#define TEST_ADDRESS_1  0x01FA
#define TEST_ADDRESS_2  0x01FB
#define TEST_ADDRESS_3  0x01FC
#define TEST_ADDRESS_4  0x01FD
#define TEST_ADDRESS_5  0x01FE
#define TEST_ADDRESS_6  0x01FF
#define TEST_ADDRESS_7  0x0200
#define TEST_ADDRESS_8  0x0201
#define TEST_ADDRESS_9  0x0202
#define TEST_ADDRESS_10  0x0203
#define TEST_ADDRESS_11  0x0204
#define TEST_ADDRESS_12  0x0205


///////////////////////
// Control variables //
///////////////////////

bool memory_in_range = true;
bool halt = false;


///////////////////////
// Disassembly table //
///////////////////////

char opcode_mneumonics[][14] =
{
"BRA rel      ", 
"BCC rel      ", 
"BCS rel      ", 
"BNE rel      ", 
"BEQ rel      ", 
"BVC rel      ", 
"BVS rel      ", 
"BMI rel      ", 
"BPL rel      ", 
"BGE rel      ", 
"BLE rel      ", 
"BLS rel      ", 
"BHI rel      ", 
"ILLEGAL     ", 
"RTN impl     ", 
"ILLEGAL     ", 

"ST abs       ", 
"PSH  ,A      ", 
"POP A,       ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"CLC impl     ", 
"SEC impl     ", 
"CLI impl     ", 
"STI impl     ", 
"SEV impl     ", 
"CLV impl     ", 
"DEX impl     ", 
"INX impl     ", 
"NOP impl     ", 
"WAI impl     ", 
"ILLEGAL     ", 

"ST abs,X     ", 
"PSH  ,s      ", 
"POP s,       ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"ADI  #       ", 
"SBI  #       ", 
"CPI  #       ", 
"ANI  #       ", 
"XRI  #       ", 
"MVI  #,B     ", 
"MVI  #,C     ", 
"MVI  #,D     ", 
"MVI  #,E     ", 
"MVI  #,L     ", 
"MVI  #,H     ", 

"ILLEGAL     ", 
"PSH  ,B      ", 
"POP B,       ", 
"JPR abs      ", 
"CCC abs      ", 
"CCS abs      ", 
"CNE abs      ", 
"CEQ abs      ", 
"CVC abs      ", 
"CVS abs      ", 
"CMI abs      ", 
"CPL abs      ", 
"CHI abs      ", 
"CLE abs      ", 
"ILLEGAL     ", 
"ILLEGAL     ", 

"ILLEGAL     ", 
"PSH  ,C      ", 
"POP C,       ", 
"TST abs      ", 
"INC abs      ", 
"DEC abs      ", 
"RCR abs      ", 
"RCL abs      ", 
"SAL abs      ", 
"ASR abs      ", 
"NOT abs      ", 
"ROL abs      ", 
"ROR abs      ", 
"ILLEGAL     ", 
"LDX  #       ", 
"LODS  #      ", 

"STOX abs     ", 
"PSH  ,D      ", 
"POP D,       ", 
"TST abs,X    ", 
"INC abs,X    ", 
"DEC abs,X    ", 
"RCR abs,X    ", 
"RCL abs,X    ", 
"SAL abs,X    ", 
"ASR abs,X    ", 
"NOT abs,X    ", 
"ROL abs,X    ", 
"ROR abs,X    ", 
"ILLEGAL     ", 
"LDX abs      ", 
"LODS abs     ", 

"STOX abs,X   ", 
"PSH  ,E      ", 
"POP E,       ", 
"TSTA A,A     ", 
"INCA A,A     ", 
"DECA A,A     ", 
"RCRA A,A     ", 
"RCLA A,A     ", 
"SALA A,A     ", 
"ASRA A,A     ", 
"NOTA A,A     ", 
"ROLA A,A     ", 
"RORA A,A     ", 
"ILLEGAL     ", 
"LDX abs,X    ", 
"LODS abs,X   ", 

"ILLEGAL     ", 
"PSH  ,L      ", 
"POP L,       ", 
"ILLEGAL     ", 
"TAS impl     ", 
"TSA impl     ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"MOVE A,A     ", 
"MOVE B,A     ", 
"MOVE C,A     ", 
"MOVE D,A     ", 
"MOVE E,A     ", 
"MOVE L,A     ", 
"MOVE H,A     ", 
"MOVE M,A     ", 

"ILLEGAL     ", 
"PSH  ,H      ", 
"POP H,       ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"SWI impl     ", 
"RTI impl     ", 
"ILLEGAL     ", 
"MOVE A,B     ", 
"MOVE B,B     ", 
"MOVE C,B     ", 
"MOVE D,B     ", 
"MOVE E,B     ", 
"MOVE L,B     ", 
"MOVE H,B     ", 
"MOVE M,B     ", 

"ADC A,B      ", 
"SBC A,B      ", 
"CMP A,B      ", 
"IOR A,B      ", 
"AND A,B      ", 
"XOR A,B      ", 
"BT A,B       ", 
"ILLEGAL     ", 
"MOVE A,C     ", 
"MOVE B,C     ", 
"MOVE C,C     ", 
"MOVE D,C     ", 
"MOVE E,C     ", 
"MOVE L,C     ", 
"MOVE H,C     ", 
"MOVE M,C     ", 

"ADC A,C      ", 
"SBC A,C      ", 
"CMP A,C      ", 
"IOR A,C      ", 
"AND A,C      ", 
"XOR A,C      ", 
"BT A,C       ", 
"ILLEGAL     ", 
"MOVE A,D     ", 
"MOVE B,D     ", 
"MOVE C,D     ", 
"MOVE D,D     ", 
"MOVE E,D     ", 
"MOVE L,D     ", 
"MOVE H,D     ", 
"MOVE M,D     ", 

"ADC A,D      ", 
"SBC A,D      ", 
"CMP A,D      ", 
"IOR A,D      ", 
"AND A,D      ", 
"XOR A,D      ", 
"BT A,D       ", 
"LD  #        ", 
"MOVE A,E     ", 
"MOVE B,E     ", 
"MOVE C,E     ", 
"MOVE D,E     ", 
"MOVE E,E     ", 
"MOVE L,E     ", 
"MOVE H,E     ", 
"MOVE M,E     ", 

"ADC A,E      ", 
"SBC A,E      ", 
"CMP A,E      ", 
"IOR A,E      ", 
"AND A,E      ", 
"XOR A,E      ", 
"BT A,E       ", 
"LD abs       ", 
"MOVE A,L     ", 
"MOVE B,L     ", 
"MOVE C,L     ", 
"MOVE D,L     ", 
"MOVE E,L     ", 
"MOVE L,L     ", 
"MOVE H,L     ", 
"MOVE M,L     ", 

"ADC A,L      ", 
"SBC A,L      ", 
"CMP A,L      ", 
"IOR A,L      ", 
"AND A,L      ", 
"XOR A,L      ", 
"BT A,L       ", 
"LD abs,X     ", 
"MOVE A,H     ", 
"MOVE B,H     ", 
"MOVE C,H     ", 
"MOVE D,H     ", 
"MOVE E,H     ", 
"MOVE L,H     ", 
"MOVE H,H     ", 
"MOVE M,H     ", 

"ADC A,H      ", 
"SBC A,H      ", 
"CMP A,H      ", 
"IOR A,H      ", 
"AND A,H      ", 
"XOR A,H      ", 
"BT A,H       ", 
"ILLEGAL     ", 
"MOVE A,M     ", 
"MOVE B,M     ", 
"MOVE C,M     ", 
"MOVE D,M     ", 
"MOVE E,M     ", 
"MOVE L,M     ", 
"MOVE H,M     ", 
"MOVE -,-     ", 

"ADC A,M      ", 
"SBC A,M      ", 
"CMP A,M      ", 
"IOR A,M      ", 
"AND A,M      ", 
"XOR A,M      ", 
"BT A,M       ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"JMP abs      ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"ILLEGAL     ", 

}; 

////////////////////////////////////////////////////////////////////////////////
//                           Simulator/Emulator (Start)                       //
////////////////////////////////////////////////////////////////////////////////

/*
	Function Name: Fetch
	Function Purpose: Called when we want to grab that current byte from memory
	Function Parameters: None
	Function Returns: BYTE
	Function Warnings: None
*/
BYTE fetch()
{
	BYTE byte = 0;

	if ((ProgramCounter >= 0) && (ProgramCounter <= MEMORY_SIZE))
	{
		memory_in_range = true;
		byte = Memory[ProgramCounter];
		ProgramCounter++;
	}
	else
	{
		memory_in_range = false;
	}
	return byte;
}

/*
	Function Name: getAddressAbsolute
	Function Purpose: Called when we want to get the absolute address
	Function Parameters: None
	Function Return: WORD, Address
	Function Warnings: None
*/
WORD getAddressAbsolute()
{
	BYTE LB, HB = 0;
	WORD address = 0;

	HB = fetch();
	LB = fetch();
	address += (WORD)((WORD)HB << 8) + LB;

	return address;
}

/*
	Function Name: getAddressAbsoluteX
	Function Purpose: Called when we want to get the absolute X address with index register
	Function Parameters: None
	Function Return: WORD, Address X
	Function Warnings: None
*/
WORD getAddressAbsoluteX()
{
	WORD address = 0;

	address += IndexRegister;
	address += getAddressAbsolute();

	return address;
}

/*
	Function Name: Set Flag Z (Zero flag)
	Function Purpose: Called when we want to set the zero flag
	Function Parameters: inReg (BYTE) - the register that will be used
	Function Returns: None Void
	Function Warnings: None
*/
void set_flag_z(BYTE inReg) {
	BYTE reg;
	reg = inReg;
	if (reg == 0) // zero set
	{
		Flags = Flags | FLAG_Z;
	}
	else
	{
		Flags = Flags & (0xFF - FLAG_Z);
	}
}

/*
	Function Name: Set Flag N (Negative flag)
	Function Purpose: Called when we want to set the negative flag
	Function Parameters: inReg (BYTE) - the register that will be used
	Function Returns: None void
	Function Warnings: None
*/
void set_flag_n(BYTE inReg)
{
	BYTE reg;
	reg = inReg;
	if ((reg & 0x80) != 0) // msbit set
	{
		Flags = Flags | FLAG_N;
	}
	else
	{
		Flags = Flags & (0xFF - FLAG_N);
	}
}

#pragma region Clear, Set Carry Flag (FLAG C)

/*
	Function Name: Clear Flag C (Carry flag)
	Function Purpose: Called when we want to clear the carry flag
	Function Parameters: None
	Function Returns: None void
	Function Warnings: None
*/
void clear_flag_c()
{
	// clear the carry flag
	Flags = Flags & (0xFF - FLAG_C);
}

/*
	Function Name: Set Flag C (Carry flag)
	Function Purpose: Called when we want to set the carry flag
	Function Parameters: reg (BYTE) - the register that will be used
	Function Returns: None void
	Function Warnings: Does not get called properly by some op codes
*/
void set_flag_c(BYTE inReg)
{
	BYTE reg;
	reg = inReg;
	if (reg >= 0x100) {
		Flags = Flags | FLAG_C; // Set carry flag
	}
	else {
		clear_flag_c(); // Clear carry flag
	}
}

#pragma endregion

/*
	Function Name: Set Flag V (Overflow flag)
	Function Purpose: Called when we want to set the Overflow flag
	Function Parameters: accepts 3 bytes
	Function Returns: None void
	Function Warnings: None
*/
void set_flag_v(BYTE in1, BYTE in2, BYTE out1)
{
	BYTE reg1in;
	BYTE reg2in;
	BYTE regOut;
	reg1in = in1;
	reg2in = in2;
	regOut = out1;

	// overflow code
	if ((((reg1in & 0x80) == 0x80) && ((reg2in & 0x80) == 0x80) && ((regOut & 0x80) != 0x80)) //overflow
		|| (((reg1in & 0x80) != 0x80) && ((reg2in & 0x80) != 0x80) && ((regOut & 0x80) == 0x80))) //overflow
	{

		Flags = Flags | FLAG_V;

	}
	else
	{

		Flags = Flags & (0xFF - FLAG_V);

	}
}

/*
	Function Name: Push
	Function Purpose: Called when we want to push a byte to the memory via stack pointer
	then we decrement it
	Function Returns: None void
	Function Warnings: None
*/
void push(BYTE reg) {
	//Move what's in a register onto the stack
	Memory[StackPointer] = reg;
	//Move the stackpointer so values aren't overwritted
	StackPointer--;
}

/*
	Function Name: BRANCH
	Function Purpose: Called when we want BRANCH
	Function Parameters: LB (WORD) data from fetch()
	Function Returns: Address (WORD)
	Function Warnings: None
*/
WORD BRANCH(WORD LB)
{
	WORD offset = 0;
	offset = (WORD)LB;

	if ((offset & 0x80) != 0)
	{
		offset = offset + 0xFF00;
	}

	return ProgramCounter + offset;
}

/*
	Function Name: CALL
	Function Purpose: Called when we want call absolute addressing
	Function Parameters: Absolute Address (WORD)
	Function Returns: ABS Address (WORD)
	Function Warnings: None
*/
WORD CALL(WORD ABS_address)
{
	//Push program counter onto the top of the stack
	push(ProgramCounter);
	//Set the program counter to have the current address
	return ABS_address;
}

/*
	Function Name: PERFORM_BIT_TEST
	Function Purpose: Used to do a bit test on a target register
	Function Parameters: targetReg (BYTE) - the register that will be used
	Function Returns: None void
	Function Warnings: None
*/
void PERFORM_BIT_TEST(BYTE targetReg)
{
	WORD temp_word;
	
	// perform bitwise and on register A and target reigster
	temp_word = (WORD)Registers[REGISTER_A] & (WORD)targetReg;

	// set flags z and n to result
	set_flag_z((WORD)temp_word);
	set_flag_n((WORD)temp_word);
}

/*
	Function Name: REGISTER_BIT_WITH_A
	Function Purpose: used to register bitwise with the accumulator
	Function Parameters: targetReg (BYTE) - the register that will be used
	Function Returns: WORD temp_word bitwise value
	Function Warnings: None
*/
WORD REGISTER_BIT_WITH_A(BYTE targetReg)
{
	WORD param1, param2, temp_word = 0;

	//Loads the paramater variables which the register contents
	param1 = Registers[REGISTER_A];
	param2 = targetReg;

	//Create a store for the paramaters once they have been and'd 
	temp_word = (WORD)param1 & (WORD)param2;

	//Set flag z and n based on the result
	set_flag_z((WORD)temp_word);
	set_flag_n((WORD)temp_word);

	return temp_word;
}

/*
	Function Name: COMPARAE_TO_ACCUMULATOR
	Function Purpose: used to compare a target register to the accumulator
	Function Parameters: targetReg (BYTE) - the register that will be used
	Function Returns: None void
	Function Warnings: None
*/
void COMPARAE_TO_ACCUMULATOR(BYTE targerReg)
{
	WORD temp_word = 0;
	WORD param1, param2 = 0;

	param1 = Registers[REGISTER_A];
	param2 = targerReg;

	temp_word = (WORD)param1 - (WORD)param2;
	if (temp_word >= 0x100)
	{
		Flags = Flags | FLAG_C;
		// Set carry flag
	}
	else
	{
		clear_flag_c();
		// Clear carry flag
	}
	set_flag_n((WORD)temp_word);
	set_flag_z((WORD)temp_word);
	set_flag_v(param1, -param2, (WORD)temp_word);
}

/*
	Function Name: SUBTRACT_REGISTER_TO_ACCUMULATOR_WITH_CARRY
	Function Purpose: Used to subtract the target register to the accumulator with the carry
	Function Parameters: targetReg (BYTE) - the register that will be used
	Function Returns: WORD temp_word result
	Function Warnings: None
*/
WORD SUBTRACT_REGISTER_TO_ACCUMULATOR_WITH_CARRY(BYTE carryFlag, WORD targetReg)
{
	WORD param1, param2, temp_word = 0;

	param1 = Registers[REGISTER_A];
	param2 = targetReg;
	temp_word = (WORD)param1 - (WORD)param2;

	if (carryFlag != 0)
	{
		temp_word--;
	}

	if (temp_word >= 0x100)
	{
		Flags = Flags | FLAG_C;
		// Set carry flag
	}
	else
	{
		clear_flag_c();
		// Clear carry flag
	}

	set_flag_n((WORD)temp_word);
	set_flag_z((WORD)temp_word);
	set_flag_v(param1, -param2, (WORD)temp_word);

	return (WORD)temp_word;
}

/*
	Function Name: ADDED_REGISTER_TO_ACCUMULATOR_WITH_CARRY
	Function Purpose: Used to add the target register to the accumulator with the carry
	Function Parameters: targetReg (BYTE) - the register that will be used
	Function Returns: WORD temp_word result
	Function Warnings: None
*/
WORD ADDED_REGISTER_TO_ACCUMULATOR_WITH_CARRY(BYTE carryFlag, WORD targetReg)
{
	WORD param1, param2, temp_word = 0;

	param1 = Registers[REGISTER_A];
	param2 = targetReg;
	temp_word = (WORD)param1 + (WORD)param2;

	if (carryFlag != 0)
	{
		temp_word++;
	}

	if (temp_word >= 0x100)
	{
		Flags = Flags | FLAG_C;
		// Set carry flag
	}
	else
	{
		clear_flag_c();
		// Clear carry flag
	}

	set_flag_n((WORD)temp_word);
	set_flag_z((WORD)temp_word);
	set_flag_v(param1, param2, (WORD)temp_word);

	return (WORD)temp_word;
}

/*
	Function Name: REGISTER_BIT_INCLUSIVE_OR_WITH_ACCUMULATOR
	Function Purpose: Used to use the target register to bitwise inclusive or with accumulator
	Function Parameters: targetReg (BYTE) - the register that will be used
	Function Returns: WORD temp_word result
	Function Warnings: None
*/
WORD REGISTER_BIT_INCLUSIVE_OR_WITH_ACCUMULATOR(BYTE targetReg)
{
	WORD temp_word = 0;

	temp_word = Registers[REGISTER_A] | targetReg;

	set_flag_z((WORD)temp_word);
	set_flag_n((WORD)temp_word);

	Flags = Flags & (0xFF - FLAG_V);

	return (WORD)temp_word;
}

WORD REGISTER_BIT_EXCLUSIVE_OR_WITH_ACCUMULATOR(BYTE targetReg)
{
	WORD temp_word = 0;

	temp_word = Registers[REGISTER_A] ^ targetReg;

	set_flag_z((WORD)temp_word);
	set_flag_n((WORD)temp_word);

	Flags = Flags & (0xFF - FLAG_V);

	return (WORD)temp_word;
}

/*
	Function Name: Group_1
	Function Purpose: Called when we want have OPCodes, used to emulator the processor commands
	Function Return: None
*/
void Group_1(BYTE opcode)
{
	//FLAGS FOR BRANCH OPERATIONS
	BYTE NF, VF, CF, ZF;

	#pragma region Setting Flag Bytes
		// sets BRANCH operator for FLAG N
		if ((Flags & FLAG_N) == 0)
		{
			NF = 0;
		}
		else 
		{
			NF = 1;
		}

		// sets BRANCH operator for FLAG V
		if ((Flags & FLAG_V) == 0)
		{
			VF = 0;
		}
		else 
		{
			VF = 1;
		}

		// sets BRANCH operator for FLAG C
		if ((Flags & FLAG_C) == 0)
		{
			CF = 0;
		}
		else 
		{
			CF = 1;
		}

		// sets BRANCH operator for FLAG Z
		if ((Flags & FLAG_Z) == 0)
		{
			ZF = 0;
		}
		else 
		{
			ZF = 1;
		}

	#pragma endregion

	// temp variables that will be used to process the other op codes
	BYTE LB = 0;
	BYTE HB = 0;
	WORD address = 0;
	WORD data = 0;
	WORD temp_word = 0;
	WORD param1 = 0;
	WORD param2 = 0;
	WORD offset = 0;
	byte saved_flags = Flags;

	// case statement where we simulate the op code commands
	switch(opcode) {

		/*	
			Week 1: 
			Practical: LD (#), LD (ABS), LD (ABS,X), ST (ABS), ST (ABS, X),
		*/

		#pragma region LD,ABS,ABS X

		/* ===============================================
			LD OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		LD Addressing #
			HEX:				0xB7	
			DESCRIPTION:		Loads Memory into Accumulator
			PARAMS:				None
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - 0 ]
			COMMENTS:			N/A
		=============================================== */
		case 0xB7: // LD (#) - Hex: 0xB7

			data = fetch();
			Registers[REGISTER_A] = data;

			set_flag_z(Registers[REGISTER_A]);
			set_flag_n(Registers[REGISTER_A]);

			// clear the carry flag
			clear_flag_c();

			break;

		/* ===============================================
			OPERATOR CODE:		LD Addressing Absolute
			HEX:				0xC7
			DESCRIPTION:		Loads Memory into Accumulator
			PARAMS:				None
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - 0 ]
			COMMENTS:			N/A
		=============================================== */
		case 0xC7: // LD (abs) - Hex: 0xC7

			address = getAddressAbsolute();

			if (address >= 0 && address < MEMORY_SIZE)
			{
				Registers[REGISTER_A] = Memory[address];
			}

			set_flag_z(Registers[REGISTER_A]);
			set_flag_n(Registers[REGISTER_A]);

			// clear the carry flag
			clear_flag_c();

			break;

		/* ===============================================
			OPERATOR CODE:		LD Adressing Absolute X
			HEX:				0xD7
			DESCRIPTION:		Loads Memory into Accumulator
			PARAMS:				None
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - 0 ]
			COMMENTS:			N/A
		=============================================== */
		case 0xD7: // LD (abs,X) - Hex: 0xD7

			address = getAddressAbsoluteX();

			if (address >= 0 && address < MEMORY_SIZE)
			{
				Registers[REGISTER_A] = Memory[address];
			}

			set_flag_z(Registers[REGISTER_A]);
			set_flag_n(Registers[REGISTER_A]);

			// clear the carry flag
			clear_flag_c();
			break;

		#pragma endregion

		#pragma region LDX, ABS, ABS X

		/* ===============================================
			LDX OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		LDX
			HEX:				0x4E
			DESCRIPTION:		Loads Memory into register X
			PARAMS:				None
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - 0 ]
			COMMENTS:			N/A
		=============================================== */
		case 0x4E: // LDX (#) 0x4E

			data = fetch();

			IndexRegister = (WORD)data;

			set_flag_n(IndexRegister);
			set_flag_z(IndexRegister);

			clear_flag_c();

			break;

		/* ===============================================
			OPERATOR CODE:		LDX Absolute
			HEX:				0x5E
			DESCRIPTION:		Loads Memory into register X
			PARAMS:				None
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - 0 ]
			COMMENTS:			N/A
		=============================================== */
		case 0x5E: // LDX (ABS) 0x5E

			address = getAddressAbsolute();

			if (address >= 0 && address < MEMORY_SIZE) {
				IndexRegister = Memory[address];
			}

			set_flag_n(IndexRegister);
			set_flag_z(IndexRegister);

			clear_flag_c();

			break;

		/* ===============================================
			OPERATOR CODE:		LDX Absolute X
			HEX:				0x6E
			DESCRIPTION:		Loads Memory into register X
			PARAMS:				None
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - 0 ]
			COMMENTS:			N/A
		=============================================== */
		case 0x6E: // LDX (ABS, X) 0x6E

			address = getAddressAbsoluteX();

			if (address >= 0 && address < MEMORY_SIZE) {
				IndexRegister = Memory[address];
			}

			set_flag_n(IndexRegister);
			set_flag_z(IndexRegister);
			clear_flag_c();

			break;

		#pragma endregion

		#pragma region ST ABS and ABS X

		/* ===============================================
			ST OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		ST Absolute
			HEX:				0x10
			DESCRIPTION:		Stores Accumulator into Memory
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - 0 ]
			COMMENTS:			N/A
		=============================================== */
		case 0x10: // ST (abs) - Hex: 0x10

			address = getAddressAbsolute();
			if (address >= 0 && address < MEMORY_SIZE) 
			{ 
				Memory[address] = Registers[REGISTER_A]; 
			}

			set_flag_n(Memory[address]);
			set_flag_z(Memory[address]);

			clear_flag_c();

			break;

		/* ===============================================
			OPERATOR CODE:		ST Absolute X
			HEX:				0x20
			DESCRIPTION:		Stores Accumulator into Memory
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - 0 ]
			COMMENTS:			N/A
		=============================================== */
		case 0x20: // ST (abs,X) - Hex: 0x20
			address = getAddressAbsoluteX();
			if (address >= 0 && address < MEMORY_SIZE)
			{
				Memory[address] = Registers[REGISTER_A];
			}

			set_flag_n(Memory[address]);
			set_flag_z(Memory[address]);

			clear_flag_c();

			break;

		#pragma endregion

		#pragma region  STOX ABS and ABS X

		/* ===============================================
			STOX OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		STOX Absolute
			HEX:				0x50
			DESCRIPTION:		Stores register X into Memory
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - 0 ]
			COMMENTS:			N/A
		=============================================== */
		case 0x50: // STOX (ABS) - Hex: 0x50
			address = getAddressAbsolute();

			if (address >= 0 && address < MEMORY_SIZE)
			{
				Memory[address] = IndexRegister;
			}

			set_flag_n(IndexRegister);
			set_flag_z(IndexRegister);

			clear_flag_c();
			break;

		/* ===============================================
			OPERATOR CODE:		STOX Absolute X
			HEX:				0x60
			DESCRIPTION:		Stores register X into Memory
			PARAMS:
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - 0 ]
			COMMENTS:			N/A
		=============================================== */
		case 0x60: // STOX (ABS, X) - Hex: 0x60
			address = getAddressAbsoluteX();
			if (address >= 0 && address < MEMORY_SIZE)
			{
				Memory[address] = IndexRegister;
			}

			set_flag_n(IndexRegister);
			set_flag_z(IndexRegister);

			clear_flag_c();
			break;

		#pragma endregion

		#pragma region MVI

		/* ===============================================
			MVI OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		MVI -> B, #
			HEX:				0x2A
			DESCRIPTION:		Loads Memory Into Register B Immediate Addressing
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - 0 ]
			COMMENTS:			N/A
		=============================================== */
		case 0x2A: // MVI (B,#)
			data = fetch(); 
			Registers[REGISTER_B] = data;
			set_flag_n(data);
			set_flag_z(data);
			clear_flag_c();
			break;

		/* ===============================================
			OPERATOR CODE:		MVI -> C, #
			HEX:				0x2B
			DESCRIPTION:		Loads Memory Into Register C Immediate Addressing
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - 0 ]
			COMMENTS:			N/A
		=============================================== */
		case 0x2B: // MVI (C,#)
			data = fetch();
			Registers[REGISTER_C] = data;
			set_flag_n(data);
			set_flag_z(data);
			clear_flag_c();
			break;

		/* ===============================================
			OPERATOR CODE:		MVI -> D, #
			HEX:				0x2C
			DESCRIPTION:		Loads Memory Into Register D Immediate Addressing
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - 0 ]
			COMMENTS:			N/A
		=============================================== */
		case 0x2C: // MVI (D,#)
			data = fetch();
			Registers[REGISTER_D] = data;
			set_flag_n(data);
			set_flag_z(data);
			clear_flag_c();
			break;

		/* ===============================================
			OPERATOR CODE:		MVI -> E, #
			HEX:				0x2D
			DESCRIPTION:		Loads Memory Into Register E Immediate Addressing
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - 0 ]
			COMMENTS:			N/A
		=============================================== */
		case 0x2D: // MVI (E,#)
			data = fetch();
			Registers[REGISTER_E] = data;
			set_flag_n(data);
			set_flag_z(data);
			clear_flag_c();
			break;

		/* ===============================================
			OPERATOR CODE:		MVI -> L, #
			HEX:				0x2E
			DESCRIPTION:		Loads Memory Into Register L Immediate Addressing
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - 0 ]
			COMMENTS:			N/A
		=============================================== */
		case 0x2E: // MVI (L,#)
			data = fetch();
			Registers[REGISTER_L] = data;
			set_flag_n(data);
			set_flag_z(data);
			clear_flag_c();
			break;

		/* ===============================================
			OPERATOR CODE:		MVI -> H, #
			HEX:				0x2F
			DESCRIPTION:		Loads Memory Into Register HL Immediate Addressing
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - 0 ]
			COMMENTS:			N/A
		=============================================== */
		case 0x2F: // MVI (H,#)
			data = fetch();
			Registers[REGISTER_H] = data;
			set_flag_n(data);
			set_flag_z(data);
			clear_flag_c();
			break;

		#pragma endregion

		#pragma region LODS

		/* ===============================================
			LODS OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		LODS #
			HEX:				0x4F
			DESCRIPTION:		Loads Memory Into Stackpointer Immediate Addressing
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - 0 ]
			COMMENTS:			N/A
		=============================================== */
		case 0x4F: // LODS (#) - Hex: 0x4F

			data = fetch();
			StackPointer = data << 8; 
			StackPointer += fetch();

			set_flag_n(Memory[data]);
			set_flag_z(Memory[data]);

			clear_flag_c();
			break;
		
		/* ===============================================
			OPERATOR CODE:		LODS Absolute
			HEX:				0x5F
			DESCRIPTION:		Loads Memory Into Stackpointer Absolute Addressing
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - 0 ]
			COMMENTS:			N/A
		=============================================== */
		case 0x5F: // LODS (abs) - Hex: 0x5F

			address = getAddressAbsolute();

			if (address >= 0 && address < MEMORY_SIZE - 1) { 
				StackPointer = (WORD)Memory[address]<< 8; 
				StackPointer += Memory[address + 1]; 
			}

			set_flag_n(Memory[address]);
			set_flag_z(Memory[address]);

			clear_flag_c();
			break;

		/* ===============================================
			OPERATOR CODE:		LODS Absolute X
			HEX:				0x6F
			DESCRIPTION:		Loads Memory Into Stackpointer Indexed Absolute Addressing
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - 0 ]
			COMMENTS:			N/A
		=============================================== */
		case 0x6F: // LODS (abs, x) - Hex: 0x6F

			address = getAddressAbsoluteX();

			if (address >= 0 && address < MEMORY_SIZE - 1) {
				StackPointer = (WORD)Memory[address] << 8; 
				StackPointer += Memory[address + 1]; 
			}

			set_flag_n(Memory[address]);
			set_flag_z(Memory[address]);

			clear_flag_c();
			break;

		#pragma endregion

		#pragma region CMP
			
		/* ===============================================
			CMP OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		CMP
			HEX:				0x92
			DESCRIPTION:		Register Compared B To Accumulator (REG_A)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T T T - - T ]
			COMMENTS:			N/A
		=============================================== */
		case 0x92: // CMP A to B
			COMPARAE_TO_ACCUMULATOR(Registers[REGISTER_B]);
			break;

		/* ===============================================
			OPERATOR CODE:		CMP
			HEX:				0xA2
			DESCRIPTION:		Register Compared C To Accumulator (REG_A)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T T T - - T ]
			COMMENTS:			N/A
		=============================================== */
		case 0xA2: // CMP A to C
			COMPARAE_TO_ACCUMULATOR(Registers[REGISTER_C]);
			break;

		/* ===============================================
			OPERATOR CODE:		CMP
			HEX:				0xB2
			DESCRIPTION:		Register Compared D To Accumulator (REG_A)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T T T - - T ]
			COMMENTS:			N/A
		=============================================== */
		case 0xB2: // CMP A to D
			COMPARAE_TO_ACCUMULATOR(Registers[REGISTER_D]);
			break;

		/* ===============================================
			OPERATOR CODE:		CMP
			HEX:				0xC2
			DESCRIPTION:		Register Compared E To Accumulator (REG_A)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T T T - - T ]
			COMMENTS:			N/A
		=============================================== */
		case 0xC2: // CMP A to E
			COMPARAE_TO_ACCUMULATOR(Registers[REGISTER_E]);
			break;

		/* ===============================================
			OPERATOR CODE:		CMP
			HEX:				0xD2
			DESCRIPTION:		Register Compared L To Accumulator (REG_A)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T T T - - T ]
			COMMENTS:			N/A
		=============================================== */
		case 0xD2: // CMP A to L
			COMPARAE_TO_ACCUMULATOR(Registers[REGISTER_L]);
			break;
			
		/* ===============================================
			OPERATOR CODE:		CMP
			HEX:				0xE2
			DESCRIPTION:		Register Compared H To Accumulator (REG_A)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T T T - - T ]
			COMMENTS:			N/A
		=============================================== */
		case 0xE2: // CMP A to H
			COMPARAE_TO_ACCUMULATOR(Registers[REGISTER_H]);
			break;
		
		/* ===============================================
			OPERATOR CODE:		CMP
			HEX:				0xF2
			DESCRIPTION:		Register Compared M To Accumulator (REG_A)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T T T - - T ]
			COMMENTS:			N/A
		=============================================== */
		case 0xF2: // CMP A to M
			COMPARAE_TO_ACCUMULATOR(Registers[REGISTER_M]);
			break;


		#pragma endregion

		#pragma region TAS & TSA

		/* ===============================================
			TAS & TSA OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		TAS impl
			HEX:				0x74
			DESCRIPTION:		Transters Accumulator to Status Register (Implied Addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x74: // TAS
			Flags = Registers[REGISTER_A];
			break;

		/* ===============================================
			OPERATOR CODE:		TSA impl
			HEX:				0x75
			DESCRIPTION:		Transters Status register to Accumulator (Implied Addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x75: // TSA
			Registers[REGISTER_A] = Flags;
			break;

		#pragma endregion

		/* 
			Week 2: Clear and Set Flags.
		*/

		#pragma region CLC,SEC,CLI,STI,CLV

		/* ===============================================
			CLC, SEC, CLI, STI & CLV OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		CLC
			HEX:				0x15
			DESCRIPTION:		Clear the carry flag (implied addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - 0 ]
			COMMENTS:			N/A
		=============================================== */
		case 0x15: // CLC clear the carry flag CLC
			clear_flag_c();
			break;

		/* ===============================================
			OPERATOR CODE:		SEC
			HEX:				0x16
			DESCRIPTION:		Set the carry flag (implied addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - 1 ]
			COMMENTS:			N/A
		=============================================== */
		case 0x16: // SEC set the carry flag
			Flags = Flags | FLAG_C;
			break;

		/* ===============================================
			OPERATOR CODE:		CLI
			HEX:				0x17
			DESCRIPTION:		Clear interupt flag (implied addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ 0 - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x17: // CLI clear the interupt flag
			Flags = Flags & (0xFF - FLAG_I);
			break;

		/* ===============================================
			OPERATOR CODE:		STI
			HEX:				0x18
			DESCRIPTION:		Set interupt flag (implied addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ 1 - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x18: // STI set the interupt flag
			Flags = Flags | FLAG_I;
			break;

		/* ===============================================
			OPERATOR CODE:		SEV
			HEX:				0x19
			DESCRIPTION:		Set overflow flag (implied addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - 1 - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x19: // SEV Set overflow flag
			Flags = Flags | FLAG_V;
			break;

		/* ===============================================
			OPERATOR CODE:		CLV
			HEX:				0x1A
			DESCRIPTION:		Clear overflow flag (implied addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - 0 - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x1A: // CLV clear the overflow flag CLC
			Flags = Flags & (0xFF - FLAG_V);
			break;

		#pragma endregion

		#pragma region NOP

		/* ===============================================
			NOP OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		NOP
			HEX:				0x1D
			DESCRIPTION:		No Operation (implied addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x1D:
			// NOP
			break;

		#pragma endregion

		#pragma region WAI

		/* ===============================================
			NOP OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		WAI
			HEX:				0x1E
			DESCRIPTION:		Wait for interupt (implied addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x1E:  // WAI
				halt = true;
			break;

		#pragma endregion

		/*	
			Week 3: Stack operations/operators
		*/

		#pragma region PSH A, FL, B, C, D, E, L, H

		/* ===============================================
			PSH OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		PSH A
			HEX:				0x11
			DESCRIPTION:		Pushes Register A onto the stack
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x11: // psh function A

			// safe gaurd measure
			if ((StackPointer >= 1) && (StackPointer < MEMORY_SIZE))
			{
				// push the accumulator
				push(Registers[REGISTER_A]);
			}
			break;

		/* ===============================================
			OPERATOR CODE:		PSH FL
			HEX:				0x21
			DESCRIPTION:		Pushes Register status register onto the stack
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x21: // psh function flags

			// safe gaurd measure
			if ((StackPointer >= 1) && (StackPointer < MEMORY_SIZE))
			{

				// push the flags
				push(Flags);
			}
			break;

		/* ===============================================
			OPERATOR CODE:		PSH B
			HEX:				0x31
			DESCRIPTION:		Pushes Register B onto the stack
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x31: // psh function B

			// safe gaurd measure
			if ((StackPointer >= 1) && (StackPointer < MEMORY_SIZE))
			{
				// push register B
				push(Registers[REGISTER_B]);
			}
			break;

		/* ===============================================
			OPERATOR CODE:		PSH C
			HEX:				0x41
			DESCRIPTION:		Pushes Register C onto the stack
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x41: // psh function C

			// safe gaurd measure
			if ((StackPointer >= 1) && (StackPointer < MEMORY_SIZE))
			{
				// push register C
				push(Registers[REGISTER_C]);
			}
			break;

		/* ===============================================
			OPERATOR CODE:		PSH D
			HEX:				0x51
			DESCRIPTION:		Pushes Register D onto the stack
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x51: // psh function D

			// safe gaurd measure
			if ((StackPointer >= 1) && (StackPointer < MEMORY_SIZE))
			{
				// push register D
				push(Registers[REGISTER_D]);
			}

			break;

		/* ===============================================
			OPERATOR CODE:		PSH E
			HEX:				0x61
			DESCRIPTION:		Pushes Register E onto the stack
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x61: // psh function E

			// safe gaurd measure
			if ((StackPointer >= 1) && (StackPointer < MEMORY_SIZE))
			{
				// push register E
				push(Registers[REGISTER_E]);
			}
			break;

		/* ===============================================
			OPERATOR CODE:		PSH L
			HEX:				0x71
			DESCRIPTION:		Pushes Register L onto the stack
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x71: // psh function L

			// safe gaurd measure
			if ((StackPointer >= 1) && (StackPointer < MEMORY_SIZE))
			{
				// push register L
				push(Registers[REGISTER_L]);
			}
			break;

		/* ===============================================
			OPERATOR CODE:		PSH H
			HEX:				0x71
			DESCRIPTION:		Pushes Register H onto the stack
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x81: // psh function H

			// safe gaurd measure
			if ((StackPointer >= 1) && (StackPointer < MEMORY_SIZE))
			{
				// push register H
				push(Registers[REGISTER_H]);
			}
			break;

		#pragma endregion

		#pragma region POP A FL, B, C, D, E, L, H

		/* ===============================================
			POP OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		POP A
			HEX:				0x12
			DESCRIPTION:		Pop the top of the stack into the register A
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x12: // POP A
			if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE)) 
			{
				StackPointer++;
				Registers[REGISTER_A] = Memory[StackPointer];
			}
			break;

		/* ===============================================
			OPERATOR CODE:		POP FL
			HEX:				0x22
			DESCRIPTION:		Pop the top of the stack into the index registers
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x22: // POP FLAGS
			if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE))
			{
				StackPointer++;
				Flags = Memory[StackPointer];
			}
			break;

		/* ===============================================
			OPERATOR CODE:		POP B
			HEX:				0x32
			DESCRIPTION:		Pop the top of the stack into the register B
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x32: // POP B
			if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE))
			{
				StackPointer++;
				Registers[REGISTER_B] = Memory[StackPointer];
			}
			break;

		/* ===============================================
			OPERATOR CODE:		POP A
			HEX:				0x42
			DESCRIPTION:		Pop the top of the stack into the register C
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x42: // POP C
			if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE))
			{
				StackPointer++;
				Registers[REGISTER_C] = Memory[StackPointer];
			}
			break;

		/* ===============================================
			OPERATOR CODE:		POP D
			HEX:				0x52
			DESCRIPTION:		Pop the top of the stack into the register D
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x52: // POP D
			if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE))
			{
				StackPointer++;
				Registers[REGISTER_D] = Memory[StackPointer];
			}
			break;

		/* ===============================================
			OPERATOR CODE:		POP E
			HEX:				0x62
			DESCRIPTION:		Pop the top of the stack into the register E
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x62: // POP E
			if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE))
			{
				StackPointer++;
				Registers[REGISTER_E] = Memory[StackPointer];
			}
			break;

		/* ===============================================
			OPERATOR CODE:		POP L
			HEX:				0x72
			DESCRIPTION:		Pop the top of the stack into the register L
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x72: // POP L
			if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE))
			{
				StackPointer++;
				Registers[REGISTER_E] = Memory[StackPointer];
			}
			break;

		/* ===============================================
			OPERATOR CODE:		POP H
			HEX:				0x82
			DESCRIPTION:		Pop the top of the stack into the register H
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x82: // POP H
			if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE))
			{
				StackPointer++;
				Registers[REGISTER_H] = Memory[StackPointer];
			}
			break;

		#pragma endregion

		#pragma region JMP

		/* ===============================================
			JMP OPERATOR CODE
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		JMP ABS
			HEX:				0xFA
			DESCRIPTION:		Loads memory into the program counter (Absolute Addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0xFA: // JMP ABS
			address = getAddressAbsolute();
			ProgramCounter = address;
			break;

		#pragma endregion

		#pragma region JPR

		/* ===============================================
			JPR OPERATOR CODE
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		JRP ABS
			HEX:				0x33
			DESCRIPTION:		Jump to subroutine (Absolute Addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x33: // JPR ABS

			address = getAddressAbsolute();

			// safe guard
			if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE)) 
			{
				Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
				StackPointer--;
			}

			ProgramCounter = address;

			break;

		#pragma endregion 

		#pragma region RTN

		/* ===============================================
			RTN OPERATOR CODE
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		RTN ABS
			HEX:				0x0E
			DESCRIPTION:		Return from subroutine (Absolute Addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x0E: // RTN
			// Increment the stackpointer
			StackPointer++;
			//Set the address to be what is on the top of the stack
			address = Memory[StackPointer];
			//Move the address into the ProgramCounter
			ProgramCounter = address;
			break;

		#pragma endregion

		#pragma region BRA, BCC, BCS, BNE, BEQ, BVC, BVS, BMI, BPL, BGE, BLE, BLS, BHI

		/* ===============================================
			BRANCH OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		BRA REL
			HEX:				0x00
			DESCRIPTION:		BRANCH Always (Offset Addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x00: // BRA op-CODE
				
			LB = fetch();
			address = BRANCH(LB);
			ProgramCounter = address;

			break;

		/* ===============================================
			OPERATOR CODE:		BCC REL
			HEX:				0x01
			DESCRIPTION:		BRANCH on carry clear (Offset Addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x01: // BCC op-CODE BRANCH on carry clear

			LB = fetch();

			if (((CF) == 0))
			{
				address = BRANCH(LB);
				ProgramCounter = address;
			}

			break;

		/* ===============================================
			OPERATOR CODE:		BCS REL
			HEX:				0x02
			DESCRIPTION:		BRANCH on carry set (Offset Addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x02: // BCS op-CODE BRANCH on carry set

			LB = fetch();

			if ((CF) != 0)
			{
				address = BRANCH(LB);
				ProgramCounter = address;
			}
			break;

		/* ===============================================
			OPERATOR CODE:		BNE REL
			HEX:				0x03
			DESCRIPTION:		BRANCH on result not zero (Offset Addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x03: // BNE BRANCH on result not zero

			LB = fetch();

			if ((ZF) == 0)
			{
				address = BRANCH(LB);
				ProgramCounter = address;
			}
			break;

		/* ===============================================
			OPERATOR CODE:		BEQ REL
			HEX:				0x04
			DESCRIPTION:		BRANCH on result equal to zero (Offset Addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x04: // BEQ BRANCH on result equal to zero

			LB = fetch();

			if ((ZF) != 0)
			{
				address = BRANCH(LB);
				ProgramCounter = address;
			}
			break;
		
		/* ===============================================
			OPERATOR CODE:		BVC REL
			HEX:				0x05
			DESCRIPTION:		BRANCH on overflow clear (Offset Addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x05: //BVC BRANCH onoverflow clear
			LB = fetch();
			if (VF == 0)
			{
				address = BRANCH(LB);
				ProgramCounter = address;
			}

			break;

		/* ===============================================
			OPERATOR CODE:		BVS REL
			HEX:				0x06
			DESCRIPTION:		BRANCH on overflow set (Offset Addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x06: //BVS BRANCH onoverflow set
			LB = fetch();
			if (VF != 0)
			{
				address = BRANCH(LB);
				ProgramCounter = address;
			}

			break;

		/* ===============================================
			OPERATOR CODE:		BMI REL
			HEX:				0x07
			DESCRIPTION:		BRANCH on negative result (Offset Addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x07: //BMI BRANCH negative result
			LB = fetch();
			if ((NF) == 1)
			{
				address = BRANCH(LB);
				ProgramCounter = address;
			}

			break;

		/* ===============================================
			OPERATOR CODE:		BPL REL
			HEX:				0x08
			DESCRIPTION:		BRANCH on positive result (Offset Addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x08: //BPL BRANCH on positive result
			LB = fetch();
			if ((NF) == 0)
			{
				address = BRANCH(LB);
				ProgramCounter = address;
			}

			break;

		/* ===============================================
			OPERATOR CODE:		BGE REL
			HEX:				0x09
			DESCRIPTION:		BRANCH on result less than or equal to zero (Offset Addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x09: //BGE BRANCH on result less than or equal to zero

			LB = fetch();
			if ((NF ^ VF) == 0) {
				address = BRANCH(LB);
				ProgramCounter = address;
			}

			break;

		/* ===============================================
			OPERATOR CODE:		BLE REL
			HEX:				0x0A
			DESCRIPTION:		BRANCH on result greater than or equal to zero (Offset Addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x0A: // BLE BRANCH on result greater than or equal to zero
			LB = fetch();
			if (((ZF) | ((NF) ^ (VF))) == 1)
			{
				address = BRANCH(LB);
				ProgramCounter = address;
			}
			break;

		/* ===============================================
			OPERATOR CODE:		BLS REL
			HEX:				0x0B
			DESCRIPTION:		BRANCH on result same or lower (Offset Addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x0B: // BLS BRANCH on result the same or lower
			LB = fetch();
			if (((CF) | (ZF)) != 0)
			{
				address = BRANCH(LB);
				ProgramCounter = address;
			}
			break;

		/* ===============================================
			OPERATOR CODE:		BHI
			HEX:				0x0C
			DESCRIPTION:		BRANCH on result higher (Offset Addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x0C: // BHI BRANCH on result higher
			LB = fetch();
			if (((CF) | (ZF)) == 0)
			{
				address = BRANCH(LB);
				ProgramCounter = address;
			}
			break;

		#pragma endregion

		#pragma region CCC, CCS, CNE, CEQ, CVC, CVS, CMI, CPL, CHI, CLE

		/* ===============================================
			CALL OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		CCC ABS
			HEX:				0x34
			DESCRIPTION:		Call on carry clear (Absolute Addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x34: // CCC call on carry clear
			address = getAddressAbsolute();

			// safe guard check
			if (CF == 0)
			{
				// call
				ProgramCounter = CALL(address);
			}
			break;

		/* ===============================================
			OPERATOR CODE:		CCS ABS
			HEX:				0x35
			DESCRIPTION:		Call on carry set (Absolute Addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x35: // CCS call carry set
			address = getAddressAbsolute();

			// safe guard check
			if (CF != 0)
			{
				// call
				ProgramCounter = CALL(address);
			}
			break;

		/* ===============================================
			OPERATOR CODE:		CNE ABS
			HEX:				0x36
			DESCRIPTION:		Call on result not zero (Absolute Addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x36: // CNE call on result not zero
			address = getAddressAbsolute();

			// safe guard check
			if (ZF == 0)
			{
				// call
				ProgramCounter = CALL(address);
			}
			break;

		/* ===============================================
			OPERATOR CODE:		CEQ ABS
			HEX:				0x37
			DESCRIPTION:		Call on result equal to zero (Absolute Addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x37: // CEQ
			address = getAddressAbsolute();

			// safe guard check
			if (ZF != 0)
			{
				// call
				ProgramCounter = CALL(address);
			}
			break;

		/* ===============================================
			OPERATOR CODE:		CVC ABS
			HEX:				0x38
			DESCRIPTION:		Call on overflow clear (Absolute Addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x38: // CVC
			address = getAddressAbsolute();

			// safe guard check
			if (VF == 0)
			{
				// call
				ProgramCounter = CALL(address);
			}
			break;

		/* ===============================================
			OPERATOR CODE:		CVS ABS
			HEX:				0x39
			DESCRIPTION:		Call on overflow set (Absolute Addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x39: //CVS

			address = getAddressAbsolute();

			// safe guard check
			if (VF == 1)
			{
				if (address >= 0 && address < MEMORY_SIZE)
				{
					if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE))
					{
						Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
						StackPointer--;
						Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
						StackPointer--;
					}
					ProgramCounter = (WORD)address;
				}
			}

			break;

		/* ===============================================
			OPERATOR CODE:		CMI ABS
			HEX:				0x3A
			DESCRIPTION:		Call on negative result (Absolute Addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x3A: // CMI call on negative result

			address = getAddressAbsolute();

			// safe guard check
			if (NF == 1)
			{
				// call
				ProgramCounter = CALL(address);
			}
			break;

		/* ===============================================
			OPERATOR CODE:		CPL ABS
			HEX:				0x3B
			DESCRIPTION:		Call on positive result (Absolute Addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x3B: // CPL call on positive result

			address = getAddressAbsolute();

			if (NF == 0)
			{
				// call
				ProgramCounter = CALL(address);
			}
			break;

		/* ===============================================
			OPERATOR CODE:		CHI ABS
			HEX:				0x3C
			DESCRIPTION:		Call on result same or lower (Absolute Addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x3C: // CHI call on result same or lower

			address = getAddressAbsolute();

			// safe guard check
			if ((CF | ZF) != 0)
			{
				// call
				ProgramCounter = CALL(address);
			}
			break;

		/* ===============================================
			OPERATOR CODE:		CLE ABS
			HEX:				0x3D
			DESCRIPTION:		Call on result higher (Absolute Addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x3D: // CLE Call on result higher

			address = getAddressAbsolute();

			// safe guard check
			if ((CF | ZF) == 0)
			{
				// call
				ProgramCounter = CALL(address);
			}
			break;

		#pragma endregion

		/*
			Week 4: 
		*/

		// INCA, DECA work, INX and DEX dont work
		#pragma region INCA, DECA, INX, DEX

		/* ===============================================
			INC AND LOGIC OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		INCA A
			HEX:				0x64
			DESCRIPTION:		Increment memory or accumulator (Addressing A)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x64: // INCA increment memory or accumulator

			// addressing a
			++Registers[REGISTER_A];

			// set flags
			set_flag_n(Registers[REGISTER_A]); 
			set_flag_z(Registers[REGISTER_A]);
	
			break;

		/* ===============================================
			OPERATOR CODE:		DECA A
			HEX:				0x65
			DESCRIPTION:		Decrement memory or accumulator (Addressing A)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x65: // DECA decrement memory or accumulator

			// addressing a
			--Registers[REGISTER_A];

			// set flags
			set_flag_n(Registers[REGISTER_A]);
			set_flag_z(Registers[REGISTER_A]);

			break;
	
		/* ===============================================
			OPERATOR CODE:		INX impl
			HEX:				0x1C
			DESCRIPTION:		Increments Register X (Implied Addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - T - - - ]
			COMMENTS:			Does not seem to function
		=============================================== */
		case 0x1C: // INX increments register X
	
			++IndexRegister;
			set_flag_z(IndexRegister);
	
			break;

		/* ===============================================
			OPERATOR CODE:		INX impl
			HEX:				0x1B
			DESCRIPTION:		Decrements Register X (Implied Addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - T - - - ]
			COMMENTS:			Does not seem to function
		=============================================== */
		case 0x1B: // DEX decrements register X

			--IndexRegister;
			set_flag_z(IndexRegister);

			break;

		#pragma endregion

		#pragma region AND

		/* ===============================================
			AND OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		AND A,B
			HEX:				0x94
			DESCRIPTION:		Register bitwise and with accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x94: //A-B
			Registers[REGISTER_A] = REGISTER_BIT_WITH_A(Registers[REGISTER_B]);
			break;
		
		/* ===============================================
			OPERATOR CODE:		AND A,C
			HEX:				0xA4
			DESCRIPTION:		Register bitwise and with accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0xA4: //A-C
			Registers[REGISTER_A] = REGISTER_BIT_WITH_A(Registers[REGISTER_C]);
			break;
		
		/* ===============================================
			OPERATOR CODE:		AND A,D
			HEX:				0xB4
			DESCRIPTION:		Register bitwise and with accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0xB4: //A-D
			Registers[REGISTER_A] = REGISTER_BIT_WITH_A(Registers[REGISTER_D]);
			break;
		
		/* ===============================================
			OPERATOR CODE:		AND A,E
			HEX:				0xC4
			DESCRIPTION:		Register bitwise and with accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0xC4: //A-E
			Registers[REGISTER_A] = REGISTER_BIT_WITH_A(Registers[REGISTER_E]);
			break;
		
		/* ===============================================
			OPERATOR CODE:		AND A,L
			HEX:				0xD4
			DESCRIPTION:		Register bitwise and with accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0xD4: //A-L
			Registers[REGISTER_A] = REGISTER_BIT_WITH_A(Registers[REGISTER_L]);
			break;
		
		/* ===============================================
			OPERATOR CODE:		AND A,H
			HEX:				0xE4
			DESCRIPTION:		Register bitwise and with accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0xE4: //A-H
			Registers[REGISTER_A] = REGISTER_BIT_WITH_A(Registers[REGISTER_H]);
			break;
		
		/* ===============================================
			OPERATOR CODE:		AND A,M
			HEX:				0xF4
			DESCRIPTION:		Register bitwise and with accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0xF4: // A-M
			Registers[REGISTER_A] = REGISTER_BIT_WITH_A(Registers[REGISTER_M]);
			break;
		
		#pragma endregion

		#pragma region BT
					
		/* ===============================================
			BT OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		BT A,B
			HEX:				0x96
			DESCRIPTION:		Register bit tested with accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x96: //BT A-B
			PERFORM_BIT_TEST(Registers[REGISTER_B]);
			break;
					
		/* ===============================================
			OPERATOR CODE:		BT A,C
			HEX:				0xA6
			DESCRIPTION:		Register bit tested with accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0xA6: //BT A-C
			PERFORM_BIT_TEST(Registers[REGISTER_C]);
			break;
					
		/* ===============================================
			OPERATOR CODE:		BT A,D
			HEX:				0xB6
			DESCRIPTION:		Register bit tested with accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0xB6: //BT A-D
			PERFORM_BIT_TEST(Registers[REGISTER_D]);
			break;
					
		/* ===============================================
			OPERATOR CODE:		BT A,E
			HEX:				0xC6
			DESCRIPTION:		Register bit tested with accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0xC6: //BT A-E
			PERFORM_BIT_TEST(Registers[REGISTER_E]);
			break;
					
		/* ===============================================
			OPERATOR CODE:		BT A,L
			HEX:				0xD6
			DESCRIPTION:		Register bit tested with accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0xD6: //BT A-L
			PERFORM_BIT_TEST(Registers[REGISTER_L]);
			break;
					
		/* ===============================================
			OPERATOR CODE:		BT A,H
			HEX:				0xE6
			DESCRIPTION:		Register bit tested with accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0xE6: //BT A-H
			PERFORM_BIT_TEST(Registers[REGISTER_H]);
			break;
					
		/* ===============================================
			OPERATOR CODE:		BT A,M
			HEX:				0xF6
			DESCRIPTION:		Register bit tested with accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0xF6: //BT A-M
			PERFORM_BIT_TEST(Registers[REGISTER_M]);
			break;
	
		#pragma endregion

		#pragma region ADI, SBI, CPI

		/* ===============================================
			ADI OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		ADI #
			HEX:				0x25
			DESCRIPTION:		Data added to the accumulator with carry
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T T T - - T ]
			COMMENTS:			N/A
		=============================================== */
		case 0x25:// ADI

			data = fetch();
			param1 = Registers[REGISTER_A];

			temp_word = (WORD)param1 + (WORD)data;

			// check for carry flag
			if (CF != 0)
			{
				temp_word++;
			}

			// set flags
			//set_flag_c(temp_word);

			if (temp_word >= 0x100) {
				Flags = Flags | FLAG_C; // Set carry flag
			}
			else {
				clear_flag_c(); // Clear carry flag
			}

			set_flag_n(temp_word);
			set_flag_z(temp_word);

			set_flag_v(param1, data, temp_word);

			// put into accumulator
			Registers[REGISTER_A] = temp_word;

			break;

		/* ===============================================
			OPERATOR CODE:		SBI #
			HEX:				0x26
			DESCRIPTION:		Data subtracted to the accumulator with carry
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T T T - - T ]
			COMMENTS:			N/A
		=============================================== */
		case 0x26:// SBI

			data = fetch();
			param1 = Registers[REGISTER_A];

			temp_word = (WORD)param1 - (WORD)data;

			// check for carry flag
			if (CF != 0)
			{
				temp_word--;
			}

			// set flags
			//set_flag_c(temp_word);

			if (temp_word >= 0x100) {
				Flags = Flags | FLAG_C; // Set carry flag
			}
			else {
				clear_flag_c(); // Clear carry flag
			}

			set_flag_n(temp_word);
			set_flag_z(temp_word);

			set_flag_v(param1, -data, temp_word);

			// put into accumulator
			Registers[REGISTER_A] = temp_word;

			break;

		/* ===============================================
			OPERATOR CODE:		CPI #
			HEX:				0x27
			DESCRIPTION:		Data compared to the accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T T T - - T ]
			COMMENTS:			N/A
		=============================================== */
		case 0x27:// CPI data compared to the accumulator

			data = fetch();
			param1 = Registers[REGISTER_A];

			temp_word = (WORD)param1 - (WORD)data;


			// set flags
			if (temp_word >= 0x100) {
				Flags = Flags | FLAG_C; // Set carry flag
			}
			else {
				clear_flag_c(); // Clear carry flag
			}

			set_flag_n(temp_word);
			set_flag_z(temp_word);

			set_flag_v(param1, -data, temp_word);

			break;

		#pragma endregion

		#pragma region INC (ABS), INC (ABS,X)

		/* ===============================================
			INC OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		INC ABS
			HEX:				0x44
			DESCRIPTION:		Increment memory or accumulator (Absolute Addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x44: // INC ABS Increment memory or accumulator
			address = getAddressAbsolute();
			Memory[address]++;

			set_flag_z(Memory[address]);
			set_flag_n(Memory[address]);

			break;

		/* ===============================================
			OPERATOR CODE:		INC ABS,X
			HEX:				0x54
			DESCRIPTION:		Increment memory or accumulator (Indexed Absolute Addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x54: // INC ABS,X

			address = getAddressAbsoluteX();
			Memory[address]++;

			set_flag_z(Memory[address]);
			set_flag_n(Memory[address]);

			break;

		#pragma endregion
		
		#pragma region DEC (ABS), DEC (ABS,X)

		/* ===============================================
			DEC OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		DEC ABS
			HEX:				0x45
			DESCRIPTION:		Decrement memory or accumulator (Absolute Addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x45: // DEC ABS decrement memory or accumulator

			address = getAddressAbsolute();
			Memory[address]--;

			set_flag_z(Memory[address]);
			set_flag_n(Memory[address]);

			break;

		/* ===============================================
			OPERATOR CODE:		DEC ABS,X
			HEX:				0x55
			DESCRIPTION:		Decrement memory or accumulator (Indexed Absolute Addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x55: // DEC ABS,X

			address = getAddressAbsoluteX();
			Memory[address]--;

			set_flag_z(Memory[address]);
			set_flag_n(Memory[address]);

			break;

		#pragma endregion
		
		#pragma region SBC
				
		/* ===============================================
			SBC OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		SBC A,B
			HEX:				0x91
			DESCRIPTION:		Register B subtracted to Accumulator with carry
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T T T - - T ]
			COMMENTS:			N/A
		=============================================== */
		case 0x91: // SBC A,B
			Registers[REGISTER_A] = SUBTRACT_REGISTER_TO_ACCUMULATOR_WITH_CARRY(CF, Registers[REGISTER_B]);
			break;
		
		/* ===============================================
			OPERATOR CODE:		SBC A,C
			HEX:				0xA1
			DESCRIPTION:		Register C subtracted to Accumulator with carry
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T T T - - T ]
			COMMENTS:			N/A
		=============================================== */
		case 0xA1: // SBC A,C
			Registers[REGISTER_A] = SUBTRACT_REGISTER_TO_ACCUMULATOR_WITH_CARRY(CF, Registers[REGISTER_C]);
			break;
			
		/* ===============================================
			OPERATOR CODE:		SBC A,D
			HEX:				0xB1
			DESCRIPTION:		Register D subtracted to Accumulator with carry
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T T T - - T ]
			COMMENTS:			N/A
		=============================================== */
		case 0xB1: // SBC A,D
			Registers[REGISTER_A] = SUBTRACT_REGISTER_TO_ACCUMULATOR_WITH_CARRY(CF, Registers[REGISTER_D]);
			break;
				
		/* ===============================================
			OPERATOR CODE:		SBC A,E
			HEX:				0xC1
			DESCRIPTION:		Register E subtracted to Accumulator with carry
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T T T - - T ]
			COMMENTS:			N/A
		=============================================== */
		case 0xC1: // SBC A,E
			Registers[REGISTER_A] = SUBTRACT_REGISTER_TO_ACCUMULATOR_WITH_CARRY(CF, Registers[REGISTER_E]);
			break;

		/* ===============================================
			OPERATOR CODE:		SBC A,L
			HEX:				0xD1
			DESCRIPTION:		Register L subtracted to Accumulator with carry
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T T T - - T ]
			COMMENTS:			N/A
		=============================================== */
		case 0xD1: // SBC A,L
			Registers[REGISTER_A] = SUBTRACT_REGISTER_TO_ACCUMULATOR_WITH_CARRY(CF, Registers[REGISTER_L]);
			break;
					
		/* ===============================================
			OPERATOR CODE:		SBC A,H
			HEX:				0xE1
			DESCRIPTION:		Register H subtracted to Accumulator with carry
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T T T - - T ]
			COMMENTS:			N/A
		=============================================== */
		case 0xE1: // SBC A,H
			Registers[REGISTER_A] = SUBTRACT_REGISTER_TO_ACCUMULATOR_WITH_CARRY(CF, Registers[REGISTER_H]);
			break;
					
		/* ===============================================
			OPERATOR CODE:		SBC A,M
			HEX:				0xF1
			DESCRIPTION:		Register M subtracted to Accumulator with carry
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T T T - - T ]
			COMMENTS:			N/A
		=============================================== */
		case 0xF1: // SBC A,M
			Registers[REGISTER_A] = SUBTRACT_REGISTER_TO_ACCUMULATOR_WITH_CARRY(CF, Registers[REGISTER_M]);
			break;
		
		#pragma endregion

		#pragma region ADC
		/* ===============================================
			ADC OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		ADC A,B
			HEX:				0x90
			DESCRIPTION:		Register B added to Accumulator with carry
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T T T - - T ]
			COMMENTS:			N/A
		=============================================== */	
		case 0x90: // ADC A,B
			Registers[REGISTER_A] = ADDED_REGISTER_TO_ACCUMULATOR_WITH_CARRY(CF,Registers[REGISTER_B]);
			break;
				
		/* ===============================================
			OPERATOR CODE:		ADC A,C
			HEX:				0xA0
			DESCRIPTION:		Register C added to Accumulator with carry
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T T T - - T ]
			COMMENTS:			N/A
		=============================================== */	
		case 0xA0: // ADC A,C
			Registers[REGISTER_A] = ADDED_REGISTER_TO_ACCUMULATOR_WITH_CARRY(CF, Registers[REGISTER_C]);
			break;

		/* ===============================================
			OPERATOR CODE:		ADC A,D
			HEX:				0xB0
			DESCRIPTION:		Register D added to Accumulator with carry
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T T T - - T ]
			COMMENTS:			N/A
		=============================================== */	
		case 0xB0: // ADC A,D
			Registers[REGISTER_A] = ADDED_REGISTER_TO_ACCUMULATOR_WITH_CARRY(CF, Registers[REGISTER_D]);
			break;
		
		/* ===============================================
			OPERATOR CODE:		ADC A,E
			HEX:				0xC0
			DESCRIPTION:		Register E added to Accumulator with carry
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T T T - - T ]
			COMMENTS:			N/A
		=============================================== */	
		case 0xC0: // ADC A,E
			Registers[REGISTER_A] = ADDED_REGISTER_TO_ACCUMULATOR_WITH_CARRY(CF, Registers[REGISTER_E]);
			break;
				
		/* ===============================================
			OPERATOR CODE:		ADC A,L
			HEX:				0xD0
			DESCRIPTION:		Register L added to Accumulator with carry
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T T T - - T ]
			COMMENTS:			N/A
		=============================================== */	
		case 0xD0: // ADC A,L
			Registers[REGISTER_A] = ADDED_REGISTER_TO_ACCUMULATOR_WITH_CARRY(CF, Registers[REGISTER_L]);
			break;
				
		/* ===============================================
			OPERATOR CODE:		ADC A,H
			HEX:				0xE0
			DESCRIPTION:		Register H added to Accumulator with carry
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T T T - - T ]
			COMMENTS:			N/A
		=============================================== */	
		case 0xE0: // ADC A,H
			Registers[REGISTER_A] = ADDED_REGISTER_TO_ACCUMULATOR_WITH_CARRY(CF, Registers[REGISTER_H]);
			break;
				
		/* ===============================================
			OPERATOR CODE:		ADC A,M
			HEX:				0xF0
			DESCRIPTION:		Register M added to Accumulator with carry
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T T T - - T ]
			COMMENTS:			N/A
		=============================================== */	
		case 0xF0: // ADC A,M
			Registers[REGISTER_A] = ADDED_REGISTER_TO_ACCUMULATOR_WITH_CARRY(CF, Registers[REGISTER_M]);
			break;
				
		#pragma endregion

		#pragma region IOR
		/* ===============================================
			IOR OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		IOR A,B
			HEX:				0x93
			DESCRIPTION:		Register B bitwise inclusive or with Accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */	
		case 0x93: // IOR A, B
			Registers[REGISTER_A] = REGISTER_BIT_INCLUSIVE_OR_WITH_ACCUMULATOR(Registers[REGISTER_B]);
			break;

		/* ===============================================
			OPERATOR CODE:		IOR A,C
			HEX:				0xA3
			DESCRIPTION:		Register C bitwise inclusive or with Accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */	
		case 0xA3: // IOR A, C
			Registers[REGISTER_A] = REGISTER_BIT_INCLUSIVE_OR_WITH_ACCUMULATOR(Registers[REGISTER_C]);
			break;

		/* ===============================================
			OPERATOR CODE:		IOR A,D
			HEX:				0xB3
			DESCRIPTION:		Register D bitwise inclusive or with Accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */		
		case 0xB3: // IOR A, D
			Registers[REGISTER_A] = REGISTER_BIT_INCLUSIVE_OR_WITH_ACCUMULATOR(Registers[REGISTER_D]);
			break;
		/* ===============================================
			OPERATOR CODE:		IOR A,E
			HEX:				0xC3
			DESCRIPTION:		Register E bitwise inclusive or with Accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */	
		case 0xC3: // IOR A, E
			Registers[REGISTER_A] = REGISTER_BIT_INCLUSIVE_OR_WITH_ACCUMULATOR(Registers[REGISTER_E]);
			break;
	
		/* ===============================================
			OPERATOR CODE:		IOR A,L
			HEX:				0xD3
			DESCRIPTION:		Register L bitwise inclusive or with Accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */	
		case 0xD3: // IOR A, L
			Registers[REGISTER_A] = REGISTER_BIT_INCLUSIVE_OR_WITH_ACCUMULATOR(Registers[REGISTER_L]);
			break;
			
		/* ===============================================
			OPERATOR CODE:		IOR A,H
			HEX:				0xE3
			DESCRIPTION:		Register H bitwise inclusive or with Accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */	
		case 0xE3: // IOR A, H
			Registers[REGISTER_A] = REGISTER_BIT_INCLUSIVE_OR_WITH_ACCUMULATOR(Registers[REGISTER_H]);
			break;
			
		/* ===============================================
			OPERATOR CODE:		IOR A,M
			HEX:				0xF3
			DESCRIPTION:		Register M bitwise inclusive or with Accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */	
		case 0xF3: // IOR A, M
			Registers[REGISTER_A] = REGISTER_BIT_INCLUSIVE_OR_WITH_ACCUMULATOR(Registers[REGISTER_M]);
			break;
			
		#pragma endregion

		#pragma region NOTA
		/* ===============================================
			NOTA OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		NOTA A
			HEX:				0x6A
			DESCRIPTION:		Negate Memory or Accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - T ]
			COMMENTS:			N/A
		=============================================== */
		case 0x6A: // NOTA

			temp_word = ~((BYTE)Registers[REGISTER_A]);
			set_flag_n(temp_word);
			set_flag_z(temp_word);
				
			// set the carry flag
			if (temp_word >= 0x100) {
				Flags = Flags | FLAG_C; // Set carry flag
			}
			else {
				clear_flag_c(); // Clear carry flag
			}

			Registers[REGISTER_A] = temp_word;

			break;

		#pragma endregion

		#pragma region NOT
		/* ===============================================
			NOT OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		NOT ABS
			HEX:				0x4A
			DESCRIPTION:		Negate Memory or Accumulator (Absolute Addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - T ]
			COMMENTS:			N/A
		=============================================== */
		case 0x4A: // NOT abs Negate Memory or Accumulator (Absolute Addressing)

			// get address
			address = getAddressAbsolute();

			// check if address is in memory range
			if (address >= 0 && address < MEMORY_SIZE)
			{
				// negate the memory address allocation and assign it to temp word
				temp_word = ~((BYTE)Memory[address]);
			}

			// set flags n and z to the byte
			set_flag_n(temp_word);
			set_flag_z(temp_word);

			// set the carry flag
			if (temp_word >= 0x100) {
				Flags = Flags | FLAG_C; // Set carry flag
			}
			else {
				clear_flag_c(); // Clear carry flag
			}

			// temp word to memory address
			Memory[address] = temp_word;

			break;

		/* ===============================================
			OPERATOR CODE:		NOT ABS X
			HEX:				0x5A
			DESCRIPTION:		Negate Memory or Accumulator (Indexed Absolute Addressing)
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - T ]
			COMMENTS:			N/A
		=============================================== */
		case 0x5A: // NOT abs, X

			address = getAddressAbsoluteX();

			if (address >= 0 && address < MEMORY_SIZE)
			{
				temp_word = ~((BYTE)Memory[address]);
			}

			set_flag_n(temp_word);
			set_flag_z(temp_word);

			// set the carry flag
			if (temp_word >= 0x100) {
				Flags = Flags | FLAG_C; // Set carry flag
			}
			else {
				clear_flag_c(); // Clear carry flag
			}

			Memory[address] = temp_word;

			break;

		#pragma endregion

		/*
			Week 5: Arithmetic Operators
		*/
		
		#pragma region XOR
		/* ===============================================
			XOR OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		XOR A, B
			HEX:				0x95
			DESCRIPTION:		Register bitwise exclusive or with Accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x95: // XOR A, B

			Registers[REGISTER_A] = REGISTER_BIT_EXCLUSIVE_OR_WITH_ACCUMULATOR(Registers[REGISTER_B]);

			break;

		/* ===============================================
			OPERATOR CODE:		XOR A, C
			HEX:				0xA5
			DESCRIPTION:		Register bitwise exclusive or with Accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0xA5: // XOR A, C

			Registers[REGISTER_A] = REGISTER_BIT_EXCLUSIVE_OR_WITH_ACCUMULATOR(Registers[REGISTER_C]);

			break;

		/* ===============================================
			OPERATOR CODE:		XOR A, D
			HEX:				0xB5
			DESCRIPTION:		Register bitwise exclusive or with Accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0xB5: // XOR A, D

			Registers[REGISTER_A] = REGISTER_BIT_EXCLUSIVE_OR_WITH_ACCUMULATOR(Registers[REGISTER_D]);

			break;

		/* ===============================================
			OPERATOR CODE:		XOR A, E
			HEX:				0xC5
			DESCRIPTION:		Register bitwise exclusive or with Accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0xC5: // XOR A, E

			Registers[REGISTER_A] = REGISTER_BIT_EXCLUSIVE_OR_WITH_ACCUMULATOR(Registers[REGISTER_E]);

			break;

		/* ===============================================
			OPERATOR CODE:		XOR A, L
			HEX:				0xD5
			DESCRIPTION:		Register bitwise exclusive or with Accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0xD5: // XOR A, L

			Registers[REGISTER_A] = REGISTER_BIT_EXCLUSIVE_OR_WITH_ACCUMULATOR(Registers[REGISTER_L]);

			break;

		/* ===============================================
			OPERATOR CODE:		XOR A, H
			HEX:				0xE5
			DESCRIPTION:		Register bitwise exclusive or with Accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0xE5: // XOR A, H

			Registers[REGISTER_A] = REGISTER_BIT_EXCLUSIVE_OR_WITH_ACCUMULATOR(Registers[REGISTER_H]);

			break;

		/* ===============================================
			OPERATOR CODE:		XOR A, M
			HEX:				0xF5
			DESCRIPTION:		Register bitwise exclusive or with Accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0xF5: // XOR A, M

			Registers[REGISTER_A] = REGISTER_BIT_EXCLUSIVE_OR_WITH_ACCUMULATOR(Registers[REGISTER_M]);

			break;

		#pragma endregion

		#pragma region ROR

		/* ===============================================
			ROR OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		ROR ABS
			HEX:				0x4C
			DESCRIPTION:		Rotate right without carry memory or Accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x4C: // ROR ABS

			address = getAddressAbsolute();

			temp_word = (Memory[address] >> 1);

			if ((Memory[address] & 0x01) != 0)
			{
				temp_word = temp_word | 0x80;
			}

			Memory[address] = (BYTE)temp_word;
			set_flag_n(Memory[address]);
			set_flag_z(Memory[address]);
			break;

		/* ===============================================
			OPERATOR CODE:		ROR ABS X
			HEX:				0x5C
			DESCRIPTION:		Rotate right without carry memory or Accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x5C: //ROR ABS,X

			address = getAddressAbsoluteX();

			temp_word = (Memory[address] >> 1);

			if ((Memory[address] & 0x01) != 0)
			{
				temp_word = temp_word | 0x80;
			}

			Memory[address] = (BYTE)temp_word;
			set_flag_n(Memory[address]);
			set_flag_z(Memory[address]);
			break;

		#pragma endregion

		#pragma region ROL

		/* ===============================================
			ROL OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		ROL ABS
			HEX:				0x4B
			DESCRIPTION:		Rotate left without carry memory or Accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x4B: // ROL ABS
			address = getAddressAbsolute();
			temp_word = (Memory[address] << 1);

			if ((Memory[address] & 0x80) == 0x80)
			{
				temp_word = temp_word | 0x01;
			}

			Memory[address] = (BYTE)temp_word;
			set_flag_n(Memory[address]);
			set_flag_z(Memory[address]);
			break;

		/* ===============================================
			OPERATOR CODE:		ROL ABS X
			HEX:				0x5B
			DESCRIPTION:		Rotate left without carry memory or Accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x5B: // ROL ABS,X
			address = getAddressAbsoluteX();
			temp_word = (Memory[address] << 1);

			if ((Memory[address] & 0x80) == 0x80)
			{
				temp_word = temp_word | 0x01;
			}

			Memory[address] = (BYTE)temp_word;
			set_flag_n(Memory[address]);
			set_flag_z(Memory[address]);
			break;

		#pragma endregion

		#pragma region RORA - A

		/* ===============================================
			RORA OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		RORA A
			HEX:				0x6C
			DESCRIPTION:		Rotate right without carry memory or accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x6C: //RORA A
			temp_word = (Registers[REGISTER_A] >> 1);

			if ((Registers[REGISTER_A] & 0x01) != 0)
			{
				temp_word = temp_word | 0x80;
			}

			Registers[REGISTER_A] = temp_word;
			set_flag_n(temp_word);
			set_flag_z(temp_word);
			break;

		#pragma endregion

		#pragma region ROLA - A
		/* ===============================================
			ROLA OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		ROLA A
			HEX:				0x6B
			DESCRIPTION:		Rotate left without carry memory or accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x6B: //ROLA A
			temp_word = (Registers[REGISTER_A] << 1);

			if ((Registers[REGISTER_A] & 0x80) == 0x80)
			{
				temp_word = temp_word | 0x01;
			}

			Registers[REGISTER_A] = temp_word;
			set_flag_n(temp_word);
			set_flag_z(temp_word);
			break;

		#pragma endregion

		#pragma region RCL ABS, ABS,X


		/* ===============================================
			RCL OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		RCL ABS
			HEX:				0x47
			DESCRIPTION:		Rotate left through carry memory or Accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - T ]
			COMMENTS:			N/A
		=============================================== */
		case 0x47: //RCL ABS
			address = getAddressAbsolute();
			saved_flags = Flags;
			if ((Memory[address] & 0x80) == 0x80) {
				Flags = Flags | FLAG_C;
			}
			else {
				clear_flag_c();
			}

			Memory[address] = (Memory[address] << 1) & 0xFE;
			if ((saved_flags & FLAG_C) == FLAG_C) {
				Memory[address] = Memory[address] | 0x01;
			}

			set_flag_n(Memory[address]);
			set_flag_z(Memory[address]);
			break;

		/* ===============================================
			OPERATOR CODE:		RCL ABS X
			HEX:				0x57
			DESCRIPTION:		Rotate right through carry memory or Accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - T ]
			COMMENTS:			N/A
		=============================================== */
		case 0x57: //RCL ABS,X
			address = getAddressAbsoluteX();
			saved_flags = Flags;
			if ((Memory[address] & 0x80) == 0x80) {
				Flags = Flags | FLAG_C;
			}
			else {
				clear_flag_c();
			}

			Memory[address] = (Memory[address] << 1) & 0xFE;
			if ((saved_flags & FLAG_C) == FLAG_C) {
				Memory[address] = Memory[address] | 0x01;
			}

			set_flag_n(Memory[address]);
			set_flag_z(Memory[address]);
			break;

		#pragma endregion

		#pragma region RCR ABS, ABS,X
		/* ===============================================
			RCR OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		RCR ABS
			HEX:				0x46
			DESCRIPTION:		Rotate right through carry memory or Accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - T ]
			COMMENTS:			N/A
		=============================================== */
		case 0x46: // RCR ABS
				address = getAddressAbsolute();
				saved_flags = Flags;
				if ((Memory[address] & 0x01) != 0) {
					Flags = Flags | FLAG_C;
				}
				else {
					clear_flag_c();
				}

				Memory[address] = (Memory[address] >> 1);
				if ((saved_flags & FLAG_C) == FLAG_C) {
					Memory[address] = Memory[address] | 0x80;
				}

				set_flag_n(Memory[address]);
				set_flag_z(Memory[address]);
			break;
		/* ===============================================
			OPERATOR CODE:		RCR ABS X
			HEX:				0x56
			DESCRIPTION:		Rotate right through carry memory or Accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - T ]
			COMMENTS:			N/A
		=============================================== */
		case 0x56: // RCR ABS, X
			address = getAddressAbsoluteX();
			saved_flags = Flags;
			if ((Memory[address] & 0x01) != 0) {
				Flags = Flags | FLAG_C;
			}
			else {
				clear_flag_c();
			}

			Memory[address] = (Memory[address] >> 1);
			if ((saved_flags & FLAG_C) == FLAG_C) {
				Memory[address] = Memory[address] | 0x80;
			}

			set_flag_n(Memory[address]);
			set_flag_z(Memory[address]);
			break;

		#pragma endregion

		#pragma region TSTA - A

		/* ===============================================
			TSTA OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		TSTA A
			HEX:				0x63
			DESCRIPTION:		Bit test memory or accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x63:
			param1 = Registers[REGISTER_A];
			temp_word = (WORD)param1 - 0x00;

			Registers[REGISTER_A] = (BYTE)temp_word;

			set_flag_n(Registers[REGISTER_A]);
			set_flag_z(Registers[REGISTER_A]);
			break;

		#pragma endregion

		#pragma region SAL and SAL ABS X
		/* ===============================================
			SAL OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		SAL ABS
			HEX:				0x48
			DESCRIPTION:		Arithmetic shift left memory or accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - T ]
			COMMENTS:			N/A
		=============================================== */
		case 0x48: // SAL ABS
			address = getAddressAbsolute();

			if (address >= 0 && address < MEMORY_SIZE)
			{
				if ((Memory[address] & 0x80) == 1)
				{
					Flags |= FLAG_C;
				}
				else
				{
					clear_flag_c();
				}
				temp_word = Memory[address] << 1;
				Memory[address] = temp_word;

				if (temp_word >= 0x100)
				{
					//set carry flag
					Flags = Flags | FLAG_C;
				}
				else
				{
					//clear carry flag
					clear_flag_c();
				}
				set_flag_z(temp_word);
				set_flag_n(temp_word);
			}

			break;

		/* ===============================================
			OPERATOR CODE:		SAL ABS X
			HEX:				0x58
			DESCRIPTION:		Arithmetic shift left memory or accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - T ]
			COMMENTS:			N/A
		=============================================== */
		case 0x58: // SAL ABS, X

			address = getAddressAbsoluteX();

			if (address >= 0 && address < MEMORY_SIZE)
			{
				if ((Memory[address] & 0x80) == 1)
				{
					Flags |= FLAG_C;
				}
				else
				{
					clear_flag_c();
				}
				temp_word = Memory[address] << 1;
				Memory[address] = temp_word;

				if (temp_word >= 0x100)
				{
					//set carry flag
					Flags = Flags | FLAG_C;
				}
				else
				{
					//clear carry flag
					clear_flag_c();
				}
				set_flag_z(temp_word);
				set_flag_n(temp_word);
			}

			break;

		#pragma endregion

		#pragma region ASR ABS, ABS, X
		/* ===============================================
			ASR OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		ASR ABS
			HEX:				0x49
			DESCRIPTION:		Arithmetic shift right memory or accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - T ]
			COMMENTS:			N/A
		=============================================== */
		case 0x49: // ASR Absolute

			address = getAddressAbsolute();
			if (address >= 0 && address < MEMORY_SIZE) 
			{
				if ((Memory[address] & 0x80) == 0x80) 
				{
					if ((Memory[address] & 0x01) != 0) 
					{
						temp_word = (Memory[address] >> 1) | 0x100;
						temp_word = (temp_word | 0x80);
					}
					else 
					{
						temp_word = Memory[address] >> 1;
						temp_word = (temp_word | 0x80);
					}
				}
				else 
				{
					if ((Memory[address] & 0x01) != 0) {
						temp_word = (Memory[address] >> 1) | 0x100;
					}
					else 
					{
						temp_word = Memory[address] >> 1;
					}
				}

				Memory[address] = temp_word;

				if (temp_word >= 0x100)
				{
					//set carry flag
					Flags = Flags | FLAG_C;
				}
				else
				{
					//clear carry flag
					clear_flag_c();
				}

				set_flag_z(temp_word);
				set_flag_n(temp_word);
			}
			break;

		/* ===============================================
			OPERATOR CODE:		ASR ABS X
			HEX:				0x59
			DESCRIPTION:		Arithmetic shift right memory or accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - T ]
			COMMENTS:			N/A
		=============================================== */
		case 0x59: // ASR Absolute X

			address = getAddressAbsoluteX();
			if (address >= 0 && address < MEMORY_SIZE)
			{
				if ((Memory[address] & 0x80) == 0x80)
				{
					if ((Memory[address] & 0x01) != 0)
					{
						temp_word = (Memory[address] >> 1) | 0x100;
						temp_word = (temp_word | 0x80);
					}
					else
					{
						temp_word = Memory[address] >> 1;
						temp_word = (temp_word | 0x80);
					}
				}
				else
				{
					if ((Memory[address] & 0x01) != 0) {
						temp_word = (Memory[address] >> 1) | 0x100;
					}
					else
					{
						temp_word = Memory[address] >> 1;
					}
				}

				Memory[address] = temp_word;

				if (temp_word >= 0x100)
				{
					//set carry flag
					Flags = Flags | FLAG_C;
				}
				else
				{
					//clear carry flag
					clear_flag_c();
				}

				set_flag_z(temp_word);
				set_flag_n(temp_word);
			}
			break;

		#pragma endregion

		#pragma region SALA

		/* ===============================================
			SALA OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		SALA A
			HEX:				0x68
			DESCRIPTION:		Arithmetic shift left memory or accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - T ]
			COMMENTS:			N/A
		=============================================== */
		case 0x68: // SALA
			saved_flags = Flags;
			if ((Registers[REGISTER_A] & 0x80) == 0x80) {
				Flags = Flags | FLAG_C;
			}
			else {
				clear_flag_c();
			}
			Registers[REGISTER_A] = (Registers[REGISTER_A] << 1) & 0xFE;

			set_flag_n(Registers[REGISTER_A]);
			set_flag_z(Registers[REGISTER_A]);

			break;

		#pragma endregion

		#pragma region ASRA

		/* ===============================================
			ASRA OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		ASRA A
			HEX:				0x69
			DESCRIPTION:		Arithmetic shift right memory or accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - T ]
			COMMENTS:			N/A
		=============================================== */

		case 0x69: // ASRA Arithmetic shift right memory or accumulator

			if ((Registers[REGISTER_A] & 0x01) == 0x01) 
			{
				Flags = Flags | FLAG_C;
			}
			else 
			{
				clear_flag_c();
			}
				
			Registers[REGISTER_A] = (Registers[REGISTER_A] >> 1) & 0x7F;

			if ((Flags & FLAG_N) == FLAG_N) 
			{
				Registers[REGISTER_A] = Registers[REGISTER_A] | 0x80;
			}

			set_flag_n(Registers[REGISTER_A]);
			set_flag_z(Registers[REGISTER_A]);

			break;

		#pragma endregion

		#pragma region RCLA
		/* ===============================================
			RCLA OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		RCLA A
			HEX:				0x67
			DESCRIPTION:		Rotate left through carry memory or Accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - T ]
			COMMENTS:			N/A
		=============================================== */
		case 0x67: // RCLA
			
			saved_flags = Flags;
			
			if ((Registers[REGISTER_A] & 0x80) == 0x80)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				clear_flag_c();
			}
			
			Registers[REGISTER_A] = (Registers[REGISTER_A] << 1) & 0xFE;
			
			if ((saved_flags & FLAG_C) == FLAG_C)
			{ 
				Registers[REGISTER_A] = Registers[REGISTER_A] | 0x01; 
			}
			
			set_flag_z(Registers[REGISTER_A]);
			set_flag_n(Registers[REGISTER_A]);
			
			break;

		#pragma endregion

		#pragma region RCRA
		/* ===============================================
			RCRA OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		RCRA A
			HEX:				0x66
			DESCRIPTION:		Rotate right through carry memory or Accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - T ]
			COMMENTS:			N/A
		=============================================== */
		case 0x66: // RCRA

			saved_flags = Flags;

			if ((Registers[REGISTER_A] & 0x01) != 0)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				clear_flag_c();
			}

			Registers[REGISTER_A] = (Registers[REGISTER_A] >> 1);

			if ((saved_flags & FLAG_C) == FLAG_C)
			{
				Registers[REGISTER_A] = Registers[REGISTER_A] | 0x80;
			}

			set_flag_z(Registers[REGISTER_A]);
			set_flag_n(Registers[REGISTER_A]);

			break;

		#pragma endregion

		#pragma region XRI
		/* ===============================================
			XRI OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		XRI
			HEX:				0x29
			DESCRIPTION:		Data bit wise exclsuive or and with accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x29: // XRI Bitwise xor with accumulator register A
			data = fetch();
			temp_word = data ^ (WORD)Registers[REGISTER_A];
			Registers[REGISTER_A] = (BYTE)temp_word;

			set_flag_n((BYTE)temp_word);
			set_flag_z((BYTE)temp_word);
			break;

		#pragma endregion
		
		#pragma region ANI
		/* ===============================================
			ANI OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		ANI
			HEX:				0x28
			DESCRIPTION:		Data bit wise and with accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x28: // ANI Data bitwise and with accumulator
			data = fetch();
			temp_word = data & (WORD)Registers[REGISTER_A];
			Registers[REGISTER_A] = (BYTE)temp_word;

			set_flag_n((BYTE)temp_word);
			set_flag_z((BYTE)temp_word);
			break;

		#pragma endregion

		#pragma region TST
		/* ===============================================
			TST OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		TST
			HEX:				0x43
			DESCRIPTION:		Bit test memory or Accumulator
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - T - T - - - ]
			COMMENTS:			N/A
		=============================================== */
		case 0x43:

			data = getAddressAbsolute();
			temp_word = Memory[data] - 0x00;
			Registers[REGISTER_A] = (BYTE)temp_word;

			set_flag_n((BYTE)temp_word);
			set_flag_z((BYTE)temp_word);
			break;

		#pragma endregion

		/*
			Software Interupt Operators
		*/

		// does not work
		#pragma region SWI
		/* ===============================================
			SWI OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		SWI
			HEX:				0x85
			DESCRIPTION:		Software Interupt
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ 1 - - - - - - - ]
			COMMENTS:			Does not function at 27.02.2020
		=============================================== */
		case 0x85: // SWI Pushes accumulator program counter status register general purpose registers in order

			// set our interupt flag
			Flags = Flags | FLAG_I;

			if ((StackPointer >= 1) && (StackPointer < MEMORY_SIZE))
			{
				// push acculumator
				push(Registers[REGISTER_A]);

				// push program counter
				push(ProgramCounter);

				// push status register
				push(Flags);

				// push general purpose registers
				push(Registers[REGISTER_B]);
				push(Registers[REGISTER_C]);
				push(Registers[REGISTER_D]);
				push(Registers[REGISTER_E]);
				push(Registers[REGISTER_L]);
				push(Registers[REGISTER_H]);

				//push(IndexRegister);
				//push(StackPointer);
			}

			break;

		#pragma endregion

		// does not work
		#pragma region RTI
		/* ===============================================
			RTI OPERATOR CODES
		=============================================== */
		/* ===============================================
			OPERATOR CODE:		RTI impl
			HEX:				0x86
			DESCRIPTION:		Return from Software Interupt
			PARAMS:				NONE
			FLAGS:				[ I - N V Z - - C ]
			FLAGS SET:			[ - - - - - - - - ]
			COMMENTS:			Does not function at 27.02.2020
		=============================================== */
		case 0x86: // RTI Pops:General purpose registers (in order)Staus register ProgramCounter Accumulator

			// Push the accumulator which is register A
			if ((StackPointer >= 1) && (StackPointer < MEMORY_SIZE)) // safe gaurd measure
			{
				StackPointer++;
				Registers[REGISTER_H] = Memory[StackPointer];
				StackPointer++;
				Registers[REGISTER_L] = Memory[StackPointer];
				StackPointer++;
				Registers[REGISTER_E] = Memory[StackPointer];
				StackPointer++;
				Registers[REGISTER_D] = Memory[StackPointer];
				StackPointer++;
				Registers[REGISTER_C] = Memory[StackPointer];
				StackPointer++;
				Registers[REGISTER_B] = Memory[StackPointer];

				StackPointer++;
				Flags = Memory[StackPointer];

				StackPointer++;
				ProgramCounter = Memory[StackPointer];

				StackPointer++;
				Registers[REGISTER_A] = Memory[StackPointer];
			}

			// clear interupt flag
			Flags = Flags & (0xFF - FLAG_I);

			break;

		#pragma endregion

	}
}

/*
	Function Name: Group_2_Move
	Function Purpose: Called when we want to transfer a register to another
*/
void Group_2_Move(BYTE opcode)
{
	int destination = 0;
	int source = 0;

	int destReg = 0;
	int sourceReg = 0;

	WORD address;

	destination = opcode & 0x0F;

	switch (destination)
	{
		case 0x08:
			destReg = REGISTER_A;
				break;

		case 0x09:
			destReg = REGISTER_B;
				break;

		case 0x0A:
			destReg = REGISTER_C;
				break;

		case 0x0B:
			destReg = REGISTER_D;
				break;

		case 0x0C:
			destReg = REGISTER_E;
				break;

		case 0x0D:
			destReg = REGISTER_L;
				break;

		case 0x0E:
			destReg = REGISTER_H;
				break;

		case 0x0F:
			destReg = REGISTER_M;
				break;
	}

	source = (opcode >> 4) & 0x0F;

	switch (source)
	{
		case 0x07:
			sourceReg = REGISTER_A;
			break;

		case 0x08:
			sourceReg = REGISTER_B;
			break;

		case 0x09:
			sourceReg = REGISTER_C;
			break;

		case 0x0A:
			sourceReg = REGISTER_D;
			break;

		case 0x0B:
			sourceReg = REGISTER_E;
			break;

		case 0x0C:
			sourceReg = REGISTER_L;
			break;

		case 0x0D:
			sourceReg = REGISTER_H;
			break;

		case 0x0E:
			sourceReg = REGISTER_M;
			break;
	}

	if (sourceReg == REGISTER_M)
	{
		address = (WORD)Registers[REGISTER_H] << 8 + (WORD)Registers[REGISTER_L];
		Registers[REGISTER_M] = Memory[address];
	}

	Registers[destReg] = Registers[sourceReg];

	if (destReg == REGISTER_M)
	{
		address = (WORD)Registers[REGISTER_H] << 8 + (WORD)Registers[REGISTER_L];
		Memory[address] = Registers[REGISTER_M];
	}
}

/*
	Function Name: Execute
	Function Purpose: Called when we want to emulate a opcode e.g. execute this op code command
*/
void execute(BYTE opcode)
{
	if (((opcode >= 0x78) && (opcode <= 0x7F))
		|| ((opcode >= 0x88) && (opcode <= 0x8F))
		|| ((opcode >= 0x98) && (opcode <= 0x9F))
		|| ((opcode >= 0xA8) && (opcode <= 0xAF))
		|| ((opcode >= 0xB8) && (opcode <= 0xBF))
		|| ((opcode >= 0xC8) && (opcode <= 0xCF))
		|| ((opcode >= 0xD8) && (opcode <= 0xDF))
		|| ((opcode >= 0xE8) && (opcode <= 0xEF)))
	{
		Group_2_Move(opcode);
	}
	else
	{
		Group_1(opcode);
	}
}

/*
	Function Name: emulate
	Function Purpose: called on load and run to begin emulation
	User Help: Also prints out debug information into the console
*/
bool showDebugInformation = false;

void emulate()
{
	BYTE opcode;
	int sanity = 0;

	ProgramCounter = 0;
	halt = false;
	memory_in_range = true;

	if (showDebugInformation)
	{
		printf("                    A  B  C  D  E  L  H  X    SP\n");
	}

	while ((!halt) && (memory_in_range) && (sanity < 500)) {

		if (showDebugInformation)
		{
			printf("%04X ", ProgramCounter);           // Print current address
		}

		opcode = fetch();
		execute(opcode);

		if (showDebugInformation)
		{
			printf("%s  ", opcode_mneumonics[opcode]);  // Print current opcode

			printf("%02X ", Registers[REGISTER_A]);
			printf("%02X ", Registers[REGISTER_B]);
			printf("%02X ", Registers[REGISTER_C]);
			printf("%02X ", Registers[REGISTER_D]);
			printf("%02X ", Registers[REGISTER_E]);
			printf("%02X ", Registers[REGISTER_L]);
			printf("%02X ", Registers[REGISTER_H]);
			printf("%04X ", IndexRegister);
			printf("%04X ", StackPointer);              // Print Stack Pointer


			if ((Flags & FLAG_I) == FLAG_I)
			{
				printf("I=1 ");
			}
			else
			{
				printf("I=0 ");
			}
			if ((Flags & FLAG_N) == FLAG_N)
			{
				printf("N=1 ");
			}
			else
			{
				printf("N=0 ");
			}
			if ((Flags & FLAG_V) == FLAG_V)
			{
				printf("V=1 ");
			}
			else
			{
				printf("V=0 ");
			}
			if ((Flags & FLAG_Z) == FLAG_Z)
			{
				printf("Z=1 ");
			}
			else
			{
				printf("Z=0 ");
			}
			if ((Flags & FLAG_C) == FLAG_C)
			{
				printf("C=1 ");
			}
			else
			{
				printf("C=0 ");
			}

			//printf("\n SANITY CHECK: %d : ", sanity);  // New line
		}
		sanity++;
	}

	printf("\n");  // New line
}

////////////////////////////////////////////////////////////////////////////////
//                            Simulator/Emulator (End)                        //
////////////////////////////////////////////////////////////////////////////////

void initialise_filenames() {
	int i;

	for (i=0; i<MAX_FILENAME_SIZE; i++) {
		hex_file [i] = '\0';
		trc_file [i] = '\0';
	}
}

int find_dot_position(char *filename) {
	int  dot_position;
	int  i;
	char chr;

	dot_position = 0;
	i = 0;
	chr = filename[i];

	while (chr != '\0') {
		if (chr == '.') {
			dot_position = i;
		}
		i++;
		chr = filename[i];
	}

	return (dot_position);
}

int find_end_position(char *filename) {
	int  end_position;
	int  i;
	char chr;

	end_position = 0;
	i = 0;
	chr = filename[i];

	while (chr != '\0') {
		end_position = i;
		i++;
		chr = filename[i];
	}

	return (end_position);
}

bool file_exists(char *filename) {
	bool exists;
	FILE *ifp;

	exists = false;

	if ( ( ifp = fopen( filename, "r" ) ) != NULL ) {
		exists = true;

		fclose(ifp);
	}

	return (exists);
}

void create_file(char *filename) {
	FILE *ofp;

	if ( ( ofp = fopen( filename, "w" ) ) != NULL ) {
		fclose(ofp);
	}
}

bool getline(FILE *fp, char *buffer) {
	bool rc;
	bool collect;
	char c;
	int  i;

	rc = false;
	collect = true;

	i = 0;
	while (collect) {
		c = getc(fp);

		switch (c) {
		case EOF:
			if (i > 0) {
				rc = true;
			}
			collect = false;
			break;

		case '\n':
			if (i > 0) {
				rc = true;
				collect = false;
				buffer[i] = '\0';
			}
			break;

		default:
			buffer[i] = c;
			i++;
			break;
		}
	}

	return (rc);
}

void load_and_run(int args,_TCHAR** argv) {
	char chr;
	int  ln;
	int  dot_position;
	int  end_position;
	long i;
	FILE *ifp;
	long address;
	long load_at;
	int  code;

	// Prompt for the .hex file

	printf("\n");
	printf("Enter the hex filename (.hex): ");

	if(args == 2){
		ln = 0;
		chr = argv[1][ln];
		while (chr != '\0')
		{
			if (ln < MAX_FILENAME_SIZE)
			{
				hex_file [ln] = chr;
				trc_file [ln] = chr;
				ln++;
			}
			chr = argv[1][ln];
		}
	} else {
		ln = 0;
		chr = '\0';
		while (chr != '\n') {
			chr = getchar();

			switch(chr) {
			case '\n':
				break;
			default:
				if (ln < MAX_FILENAME_SIZE)	{
					hex_file [ln] = chr;
					trc_file [ln] = chr;
					ln++;
				}
				break;
			}
		}

	}
	// Tidy up the file names

	dot_position = find_dot_position(hex_file);
	if (dot_position == 0) {
		end_position = find_end_position(hex_file);

		hex_file[end_position + 1] = '.';
		hex_file[end_position + 2] = 'h';
		hex_file[end_position + 3] = 'e';
		hex_file[end_position + 4] = 'x';
		hex_file[end_position + 5] = '\0';
	} else {
		hex_file[dot_position + 0] = '.';
		hex_file[dot_position + 1] = 'h';
		hex_file[dot_position + 2] = 'e';
		hex_file[dot_position + 3] = 'x';
		hex_file[dot_position + 4] = '\0';
	}

	dot_position = find_dot_position(trc_file);
	if (dot_position == 0) {
		end_position = find_end_position(trc_file);

		trc_file[end_position + 1] = '.';
		trc_file[end_position + 2] = 't';
		trc_file[end_position + 3] = 'r';
		trc_file[end_position + 4] = 'c';
		trc_file[end_position + 5] = '\0';
	} else {
		trc_file[dot_position + 0] = '.';
		trc_file[dot_position + 1] = 't';
		trc_file[dot_position + 2] = 'r';
		trc_file[dot_position + 3] = 'c';
		trc_file[dot_position + 4] = '\0';
	}

	if (file_exists(hex_file)) {
		// Clear Registers and Memory

		Registers[REGISTER_A] = 0;
		Registers[REGISTER_B] = 0;
		Registers[REGISTER_C] = 0;
		Registers[REGISTER_D] = 0;
		Registers[REGISTER_E] = 0;
		Registers[REGISTER_L] = 0;
		Registers[REGISTER_H] = 0;
		IndexRegister = 0;
		Flags = 0;
		ProgramCounter = 0;
		StackPointer = 0;

		for (i=0; i<MEMORY_SIZE; i++) {
			Memory[i] = 0x00;
		}

		// Load hex file

		if ( ( ifp = fopen( hex_file, "r" ) ) != NULL ) {
			printf("Loading file...\n\n");

			load_at = 0;

			while (getline(ifp, InputBuffer)) {
				if (sscanf(InputBuffer, "L=%x", &address) == 1) {
					load_at = address;
				} else if (sscanf(InputBuffer, "%x", &code) == 1) {
					if ((load_at >= 0) && (load_at <= MEMORY_SIZE)) {
						Memory[load_at] = (BYTE)code;
					}
					load_at++;
				} else {
					printf("ERROR> Failed to load instruction: %s \n", InputBuffer);
				}
			}

			fclose(ifp);
		}

		// Emulate

		emulate();
	} else {
		printf("\n");
		printf("ERROR> Input file %s does not exist!\n", hex_file);
		printf("\n");
	}
}

void building(int args,_TCHAR** argv){
	char buffer[1024];
	load_and_run(args,argv);
	sprintf(buffer, "0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X", 
		Memory[TEST_ADDRESS_1],
		Memory[TEST_ADDRESS_2],
		Memory[TEST_ADDRESS_3],
		Memory[TEST_ADDRESS_4], 
		Memory[TEST_ADDRESS_5],
		Memory[TEST_ADDRESS_6], 
		Memory[TEST_ADDRESS_7],
		Memory[TEST_ADDRESS_8], 
		Memory[TEST_ADDRESS_9], 
		Memory[TEST_ADDRESS_10],
		Memory[TEST_ADDRESS_11],
		Memory[TEST_ADDRESS_12]
		);
	sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR *)&server_addr, sizeof(SOCKADDR));
}

void test_and_mark() {
	char buffer[1024];
	bool testing_complete;
	int  len = sizeof(SOCKADDR);
	char chr;
	int  i;
	int  j;
	bool end_of_program;
	long address;
	long load_at;
	int  code;
	int  mark;
	int  passed;

	printf("\n");
	printf("Automatic Testing and Marking\n");
	printf("\n");

	testing_complete = false;

	sprintf(buffer, "Test Student %s", STUDENT_NUMBER);
	sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR *)&server_addr, sizeof(SOCKADDR));

	while (!testing_complete) {
		memset(buffer, '\0', sizeof(buffer));

		if (recvfrom(sock, buffer, sizeof(buffer)-1, 0, (SOCKADDR *)&client_addr, &len) != SOCKET_ERROR) {
			printf("Incoming Data: %s \n", buffer);

			//if (strcmp(buffer, "Testing complete") == 1)
			if (sscanf(buffer, "Testing complete %d", &mark) == 1) {
				testing_complete = true;
				printf("Current mark = %d\n", mark);

			}else if (sscanf(buffer, "Tests passed %d", &passed) == 1) {
				//testing_complete = true;
				printf("Passed = %d\n", passed);

			} else if (strcmp(buffer, "Error") == 0) {
				printf("ERROR> Testing abnormally terminated\n");
				testing_complete = true;
			} else {
				// Clear Registers and Memory

		Registers[REGISTER_A] = 0;
		Registers[REGISTER_B] = 0;
		Registers[REGISTER_C] = 0;
		Registers[REGISTER_D] = 0;
		Registers[REGISTER_E] = 0;
		Registers[REGISTER_L] = 0;
		Registers[REGISTER_H] = 0;
		IndexRegister = 0;
				Flags = 0;
				ProgramCounter = 0;
				StackPointer = 0;
				for (i=0; i<MEMORY_SIZE; i++) {
					Memory[i] = 0;
				}

				// Load hex file

				i = 0;
				j = 0;
				load_at = 0;
				end_of_program = false;
				FILE *ofp;
				fopen_s(&ofp ,"BRANCH.txt", "a");

				while (!end_of_program) {
					chr = buffer[i];
					switch (chr) {
					case '\0':
						end_of_program = true;

					case ',':
						if (sscanf(InputBuffer, "L=%x", &address) == 1) {
							load_at = address;
						} else if (sscanf(InputBuffer, "%x", &code) == 1) {
							if ((load_at >= 0) && (load_at <= MEMORY_SIZE)) {
								Memory[load_at] = (BYTE)code;
								fprintf(ofp, "%02X\n", (BYTE)code);
							}
							load_at++;
						} else {
							printf("ERROR> Failed to load instruction: %s \n", InputBuffer);
						}
						j = 0;
						break;

					default:
						InputBuffer[j] = chr;
						j++;
						break;
					}
					i++;
				}
				fclose(ofp);
				// Emulate

				if (load_at > 1) {
					emulate();
					// Send and store results
					sprintf(buffer, "%02X%02X %02X%02X %02X%02X %02X%02X %02X%02X %02X%02X", 
						Memory[TEST_ADDRESS_1],
						Memory[TEST_ADDRESS_2],
						Memory[TEST_ADDRESS_3],
						Memory[TEST_ADDRESS_4], 
						Memory[TEST_ADDRESS_5],
						Memory[TEST_ADDRESS_6], 
						Memory[TEST_ADDRESS_7],
						Memory[TEST_ADDRESS_8], 
						Memory[TEST_ADDRESS_9], 
						Memory[TEST_ADDRESS_10],
						Memory[TEST_ADDRESS_11],
						Memory[TEST_ADDRESS_12]
						);
					sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR *)&server_addr, sizeof(SOCKADDR));
				}
			}
		}
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	char chr;
	char dummy;

	printf("\n");
	printf("Microprocessor Emulator\n");
	printf("UWE Computer and Network Systems Assignment 1\n");
	printf("\n");

	initialise_filenames();

	if (WSAStartup(MAKEWORD(2, 2), &data) != 0) return(0);

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  // Here we create our socket, which will be a UDP socket (SOCK_DGRAM).
	if (!sock) {	
		// Creation failed! 
	}

	memset(&server_addr, 0, sizeof(SOCKADDR_IN));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS_SERVER);
	server_addr.sin_port = htons(PORT_SERVER);

	memset(&client_addr, 0, sizeof(SOCKADDR_IN));
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	client_addr.sin_port = htons(PORT_CLIENT);

	chr = '\0';
	while ((chr != 'e') && (chr != 'E'))
	{
		printf("\n");
		printf("Please select option\n");
		printf("L - Load and run a hex file\n");
		printf("T - Have the server test and mark your emulator\n");
		printf("E - Exit\n");
		if(argc == 2){ building(argc,argv); exit(0);}
		printf("Enter option: ");
		chr = getchar();
		if (chr != 0x0A)
		{
			dummy = getchar();  // read in the <CR>
		}
		printf("\n");

		switch (chr)
		{
		case 'L':
		case 'l':
			load_and_run(argc,argv);
			break;

		case 'T':
		case 't':
			test_and_mark();
			break;

		default:
			break;
		}
	}

	closesocket(sock);
	WSACleanup();


	return 0;
}


