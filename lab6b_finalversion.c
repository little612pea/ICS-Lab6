#include<stdio.h>
#include<stdlib.h>

unsigned short MEMORY[0xFFFF];
short REGISTER[8]; // 初始化寄存器
unsigned short PC; //PC
int NZP[3] = {0, 1, 0}; // 初始状态下状态码为Z

//从输入中载入指令
void FetchCode();
//执行代码
void Execute(unsigned short opcode, unsigned short INS);
void BR(unsigned short INS);
void ADD_AND(unsigned short INS, unsigned short opcode);
void LD_ST(unsigned short INS, unsigned short opcode);
void LDR_STR(unsigned short INS, unsigned short opcode);
void LDI_STI(unsigned short INS, unsigned short opcode);
void JSR_JSRR(unsigned short INS);
void NOT(unsigned short INS);
void JMP(unsigned short INS);
void LEA(unsigned short INS);
//有符号的立即数扩展
short GetImmorOffset(unsigned short instruction, int len);
//无符号的寄存器提取
unsigned short GetRegNum(unsigned short instruction, int index);
//通过读取给定寄存器重置状态码
void ResetCC(unsigned short DR);

int main()
{
    int i;
    unsigned short INS = 0; // 传入的是unsigned short类型的命令
    unsigned short opcode = 0; // 操作码
    for (i = 0; i < 0xFFFF; i++) MEMORY[i] = 0x7777;
    for (i = 0; i < 8; i++) REGISTER[i] = 0x7777;
    FetchCode(); // 从输入中载入指令
    while (1) {
        INS = MEMORY[PC++];
        opcode = INS >> 12; // 获取操作码
        if (opcode == 0b1111) break; // 遇到HALT退出循环
        Execute(opcode, INS);
    }
    for (int i = 0; i < 8; i++) printf("R%d = x%04hX\n", i, REGISTER[i]);
    return 0;
}

void FetchCode(){ // 从输入中载入指令
    char ch;
    unsigned short shortNum, addr = 0;
    int cnt;
    while(1){
        shortNum = 0, cnt = 0;
        while(cnt < 16){ // 每次只需读入16位的01串
            ch = getchar();
            if(ch == EOF) break;
            else if(ch == '0' || ch == '1'){
                shortNum += (ch - '0') * (1 << (15 - cnt));
                cnt++;
            } // 这一步没法封装成函数，因为在类型为short的情况下没有适当的返回值来表示已经读完
        }
        if(cnt < 16) break;
        if(!addr) addr = PC = shortNum; // 第一次读入的指令地址作为PC
        else MEMORY[addr++] = shortNum;
    }
}

//执行指令
void Execute(unsigned short opcode, unsigned short INS){
    switch (opcode){ //这里和我的lab6a的处理方法相似，都合并了同型指令
        case 0b0000:            BR(INS);                    break;   // BR  
        case 0b0001: 
        case 0b0101:            ADD_AND(INS, opcode);       break;  // ADD AND
        case 0b0010:
        case 0b0011:            LD_ST(INS, opcode);         break;  // LD ST
        case 0b0100:            JSR_JSRR(INS);              break;  // JSR JSRR
        case 0b0110: 
        case 0b0111:            LDR_STR(INS, opcode);       break; // LDR STR
        case 0b1001:            NOT(INS);                   break; // NOT
        case 0b1010:
        case 0b1011:            LDI_STI(INS, opcode);       break; // LDI STI
        case 0b1100:            JMP(INS);                   break; // JMP   
        case 0b1110:            LEA(INS);                   break; // LEA
        default: break;
    }
    return;
}

void BR(unsigned short INS){
    if((INS & 0x0800) && NZP[0] || // 检查N位
    (INS & 0x0400) && NZP[1] || // 检查Z位
    (INS & 0x0200) && NZP[2]) // 检查P位
    PC += GetImmorOffset(INS, 9);
}

void ADD_AND(unsigned short INS, unsigned short opcode){
    unsigned short DR = GetRegNum(INS, 9), SR1 = GetRegNum(INS, 6);
    if(INS & 0b100000) {
        short offset5 = GetImmorOffset(INS, 5);
        if(opcode == 0b0001)REGISTER[DR] = REGISTER[SR1] + offset5;
        else REGISTER[DR] = REGISTER[SR1] & offset5;
    }else {
        unsigned short SR2 = GetRegNum(INS, 0);
        if(opcode == 0b0001)REGISTER[DR] = REGISTER[SR1] + REGISTER[SR2];
        else REGISTER[DR] = REGISTER[SR1] & REGISTER[SR2];
    }
    ResetCC(DR);
}

void LD_ST(unsigned short INS, unsigned short opcode){
    unsigned short DR = GetRegNum(INS, 9), offset9 = GetImmorOffset(INS, 9);
    if(opcode == 0b0010){
        REGISTER[DR] = MEMORY[(unsigned short)(PC + offset9)];
        ResetCC(DR);
    }else MEMORY[(unsigned short)(PC + offset9)] = REGISTER[DR];
}

void JSR_JSRR(unsigned short INS){
    REGISTER[7] = PC;
    if(INS & 0x0800) PC += GetImmorOffset(INS, 11); // JSR
    else PC = REGISTER[GetRegNum(INS, 6)]; //JSRR
}

void LDR_STR(unsigned short INS, unsigned short opcode){
    unsigned short DR = GetRegNum(INS, 9), BaseR = GetRegNum(INS, 6), offset6 = GetImmorOffset(INS, 6);
    if(opcode == 0b0110){
        REGISTER[DR] = MEMORY[(unsigned short)(REGISTER[BaseR] + offset6)];
        ResetCC(DR);
    }else MEMORY[(unsigned short)(REGISTER[BaseR] + offset6)] = REGISTER[DR];
}

void NOT(unsigned short INS){
    unsigned short DR = GetRegNum(INS, 9), SR = GetRegNum(INS, 6);
    REGISTER[DR] = ~REGISTER[SR];
    ResetCC(DR);
}

void LDI_STI(unsigned short INS, unsigned short opcode){
    unsigned short DR = GetRegNum(INS, 9), offset9 = GetImmorOffset(INS, 9);
    if(opcode == 0b1010){
        REGISTER[DR] = MEMORY[MEMORY[(unsigned short)(PC + offset9)]];
        ResetCC(DR);
    }else MEMORY[MEMORY[(unsigned short)(PC + offset9)]] = REGISTER[DR];
}

void JMP(unsigned short INS){
    PC = REGISTER[GetRegNum(INS, 6)];
}

void LEA(unsigned short INS){
    unsigned short DR = GetRegNum(INS, 9), offset9 = GetImmorOffset(INS, 9);
    REGISTER[DR] = PC + offset9;
}

//有符号的立即数扩展，用short类型实现
short GetImmorOffset(unsigned short instruction, int len)
{
    short offset = instruction & ((1 << len) - 1);
    if (offset >> (len - 1)) offset |= (0xFFFF << len); // 有符号扩展
    return offset;
}

//无符号的寄存器提取，用unsigned short类型实现
unsigned short GetRegNum(unsigned short instruction, int pos)
{
    return (instruction >> pos) & 0b111;
}

//通过读取给定寄存器的值返回更改后的NZP
void ResetCC(unsigned short DR){
    NZP[0] = NZP[1] = NZP[2] = 0;
    if(REGISTER[DR] >> 15) NZP[0] = 1;
    else if(!REGISTER[DR]) NZP[1] = 1;
    else NZP[2] = 1;
}