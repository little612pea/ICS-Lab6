#include<stdio.h>

unsigned short MEMORY[0xFFFF];  // 内存中的某些内容为指令，不用来进行加减运算
short REGISTER[8]; // 寄存器的值有时会拿来做运算
unsigned short PC; // PC，其本质上是无符号的，但是要和有符号的OFFSET进行加减
int NZP[3] = {0, 1, 0}; // 初始状态下状态码为Z


void FetchCode(); // 从输入中载入指令
void BR(unsigned short INS); // BR
void ADD(unsigned short INS); // ADD
void LD(unsigned short INS); // LD
void ST(unsigned short INS); // ST
void JSR(unsigned short INS); // JSR
void AND(unsigned short INS); // AND
void LDR(unsigned short INS); // LDR
void STR(unsigned short INS); // STR
void NOT(unsigned short INS); // NOT
void LDI(unsigned short INS); // LDI
void STI(unsigned short INS); // STI
void JMP(unsigned short INS); // JMP
void LEA(unsigned short INS); // LEA
void ResetCC(short REG_DR); // 重置状态码
short GetRegNum(unsigned short INS, int pos); // 获取寄存器编号
short GetImmorOffset(unsigned short INS, int len); // 获取立即数或偏移量

int main(){
    int i;
    unsigned short opcode = 0, INS = 0;
    for(i = 0; i < 0xFFFF; i++) MEMORY[i] = 0x7777;
    for(i = 0; i < 8; i++) REGISTER[i] = 0x7777;
    FetchCode(); // 从输入中载入指令
    while(1){
        INS = MEMORY[PC++]; // PC立刻增量
        opcode = INS >> 12; // 获取操作码
        if(opcode == 0b1111) break; // 读到HALT退出循环
        switch (opcode)
        {
            case 0b0000:    BR(INS);    break;
            case 0b0001:    ADD(INS);   break;
            case 0b0010:    LD(INS);    break;
            case 0b0011:    ST(INS);    break;
            case 0b0100:    JSR(INS);   break;
            case 0b0101:    AND(INS);   break;
            case 0b0110:    LDR(INS);   break;
            case 0b0111:    STR(INS);   break;
            case 0b1001:    NOT(INS);   break;
            case 0b1010:    LDI(INS);   break;
            case 0b1011:    STI(INS);   break;
            case 0b1100:    JMP(INS);   break;
            case 0b1110:    LEA(INS);   break;
            default:                    break;
        }
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

void BR(unsigned short INS){ // BR
    short PCoffset9 = GetImmorOffset(INS, 9);
    if((INS & 0x0800) && NZP[0] || // 检查N位
    (INS & 0x0400) && NZP[1] || // 检查Z位
    (INS & 0x0200) && NZP[2]) // 检查P位
    PC += PCoffset9; // 这是个很矛盾的地方，PC本是无符号的，但是要和有符号的OFFSET进行加减，只能定义为有符号的
}

void ADD(unsigned short INS){ // ADD
    short DR = GetRegNum(INS, 9), SR1 = GetRegNum(INS, 6);
    if(INS & 0b100000) // IMM
        REGISTER[DR] = REGISTER[SR1] + GetImmorOffset(INS, 5);
    else{ // REG
        short SR2 = GetRegNum(INS, 0);
        REGISTER[DR] = REGISTER[SR1] + REGISTER[SR2];
    }
    ResetCC(DR);
}

void LD(unsigned short INS){ // LD
    short DR = GetRegNum(INS, 9), PCoffset9 = GetImmorOffset(INS, 9);
    REGISTER[DR] = MEMORY[(unsigned short)(PC + PCoffset9)]; // 计算出的PC被自动转换为有符号的，数组下标非负，所以强制转换为无符号的
    ResetCC(DR);
}

void ST(unsigned short INS){ // ST
    short SR = GetRegNum(INS, 9), PCoffset9 = GetImmorOffset(INS, 9);
    MEMORY[(unsigned short)(PC + PCoffset9)] = REGISTER[SR];
}

void JSR(unsigned short INS){ // JSR & JSRR
    short PCoffset11 = GetImmorOffset(INS, 11);
    REGISTER[7] = PC;
    if(INS & 0x0800) // JSRR
        PC = REGISTER[GetRegNum(INS, 6)];
    else PC += PCoffset11; // JSR
}

void AND(unsigned short INS){ // AND
    short DR = GetRegNum(INS, 9), SR1 = GetRegNum(INS, 6);
    if(INS & 0b100000) // IMM
        REGISTER[DR] = REGISTER[SR1] & GetImmorOffset(INS, 5);
    else{ // REG
        short SR2 = GetRegNum(INS, 0);
        REGISTER[DR] = REGISTER[SR1] & REGISTER[SR2];
    }
    ResetCC(DR);
}

void LDR(unsigned short INS){ // LDR
    short DR = GetRegNum(INS, 9), BaseR = GetRegNum(INS, 6), PCoffset6 = GetImmorOffset(INS, 6);
    REGISTER[DR] = MEMORY[(unsigned short)(REGISTER[BaseR] + PCoffset6)];
    ResetCC(DR);
}

void STR(unsigned short INS){ // STR
    short SR = GetRegNum(INS, 9), BaseR = GetRegNum(INS, 6), PCoffset6 = GetImmorOffset(INS, 6);
    MEMORY[(unsigned short)(REGISTER[BaseR] + PCoffset6)] = REGISTER[SR];
}

void NOT(unsigned short INS){ // NOT
    short DR = GetRegNum(INS, 9), SR = GetRegNum(INS, 6);
    REGISTER[DR] = ~REGISTER[SR];
    ResetCC(DR);
}

void LDI(unsigned short INS){ // LDI
    short DR = GetRegNum(INS, 9), PCoffset9 = GetImmorOffset(INS, 9);
    REGISTER[DR] = MEMORY[MEMORY[(unsigned short)(PC + PCoffset9)]];
    ResetCC(DR);
}

void STI(unsigned short INS){ // STI
    short SR = GetRegNum(INS, 9), PCoffset9 = GetImmorOffset(INS, 9);
    MEMORY[MEMORY[(unsigned short)(PC + PCoffset9)]] = REGISTER[SR];
}

void JMP(unsigned short INS){ // JMP
    PC = REGISTER[GetRegNum(INS, 6)];
}

void LEA(unsigned short INS){ // LEA
    short DR = GetRegNum(INS, 9), PCoffset9 = GetImmorOffset(INS, 9);
    REGISTER[DR] = PC + PCoffset9;
    ResetCC(DR);
}

void ResetCC(short REG_DR){ // 重置状态码
    NZP[0] = NZP[1] = NZP[2] = 0; // 清空状态码
    if(REGISTER[REG_DR] < 0) NZP[0] = 1; // 检查N位
    else if(REGISTER[REG_DR] == 0) NZP[1] = 1; // 检查Z位
    else NZP[2] = 1; // 检查P位
}

short GetRegNum(unsigned short INS, int pos){ // 获取寄存器编号
    return (INS & (0b111 << pos)) >> pos;
} // 该函数类型本该定义为unsigned short，但是执行指令的函数中short类型实在太多，方便起见，统一定义为short

short GetImmorOffset(unsigned short INS, int len){ // 获取给定长度的立即数或偏移量
    short offset = INS & ((1 << len) - 1);
    if (offset >> (len - 1)) offset |= (0xFFFF << len); // 有符号扩展
    return offset;
}