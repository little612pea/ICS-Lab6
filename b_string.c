#include<stdio.h>
#include<string.h>

char MEMORY[0xffff][17];
char REGISTER[8][17];
unsigned short PC;
char CC[3] = {'0', '1', '0'}; //N Z P

unsigned short StrtoBin(char *str, int len, int type){//type: 0表示str表示的数无符号，1表示有符号
    int i;
    unsigned short bin = 0;
    if(type == 0){
        for(i = 0; i < len; i++){
            bin = bin * 2 + str[i] - '0';
        }
    }else{
        if(str[0] == '0'){
            for(i = 1; i < len; i++){
                bin = bin * 2 + str[i] - '0';
            }
        }else{
            for(i = 1; i < len; i++){
                bin = bin * 2 + 1 - (str[i] - '0');
            }
            bin = -bin - 1;
        }
    }
    return bin;
}

char *BintoStr(char *str, unsigned short bin, int len, int type){//type: 0表示str表示的数无符号，1表示有符号，len表示str的长度
    int i;
    if(type == 0){
        for(i = len - 1; i >= 0; i--){
            str[i] = bin % 2 + '0';
            bin /= 2;
        }
    //如果是无符号数，直接转换为二进制字符串
    }else{
        if(bin >= 0){
            for(i = len - 1; i >= 0; i--){
                str[i] = bin % 2 + '0';
                bin /= 2;
            }
        }else{//如果是有符号数，转换为有符号的字符串，所有高位都要用1补齐
            bin = -bin - 1;
            for(i = len - 1; i >= 0; i--){
                str[i] = 1 - bin % 2 + '0';
                bin /= 2;
            }
            str[0] = '1';
        }
    }
    str[len] = '\0';
    return str;
}

void Readin(){
    char str[16];
    unsigned short index;
    scanf("%s", str);
    index = StrtoBin(str, 16, 0);
    PC = index;
    while(scanf("%s", str) != EOF){
        strcpy(MEMORY[index], str);
        index++;
    }
}

void ResetCC(short DR_num){
    if(DR_num == 0) CC[0] = '0', CC[1] = '1', CC[2] = '0';
    else if(DR_num > 0) CC[0] = '0', CC[1] = '0', CC[2] = '1';
    else CC[0] = '1', CC[1] = '0', CC[2] = '0';
}

void BR(char *INS){
    short PCoffset = StrtoBin(INS + 7, 9, 1);
    if(INS[4] == '1' && CC[0] == '1' || 
        INS[5] == '1' && CC[1] == '1' ||
        INS[6] == '1' && CC[2] == '1') 
        PC += PCoffset;
    PC = (unsigned short)PC;
}

void ADD(char *INS){
    unsigned short DR, SR1, SR2;
    short imm5, DR_num, SR1_num, SR2_num;
    DR = StrtoBin(INS + 4, 3, 0);
    SR1 = StrtoBin(INS + 7, 3, 0);
    SR1_num = StrtoBin(REGISTER[SR1], 16, 1);
    if(INS[10] == '0'){
        SR2 = StrtoBin(INS + 13, 3, 0);
        SR2_num = StrtoBin(REGISTER[SR2], 16, 1);
    }else{
        imm5 = StrtoBin(INS + 11, 5, 1);
        SR2_num = imm5;
    }
    DR_num = SR1_num + SR2_num;
    BintoStr(REGISTER[DR], DR_num, 16, 1);
    ResetCC(DR_num);
}

void LD(char *INS){
    unsigned short DR, mem_addr;
    short PCoffset, DR_num;
    DR = StrtoBin(INS + 4, 3, 0);
    PCoffset = StrtoBin(INS + 7, 9, 1);
    mem_addr = PC + PCoffset;
    mem_addr = (unsigned short)mem_addr;
    DR_num = StrtoBin(MEMORY[mem_addr], 16, 1);
    BintoStr(REGISTER[DR], DR_num, 16, 1);
    ResetCC(DR_num);
}

void ST(char *INS){
    unsigned short SR, mem_addr;
    short PCoffset, SR_num;
    SR = StrtoBin(INS + 4, 3, 0);
    PCoffset = StrtoBin(INS + 7, 9, 1);
    mem_addr = PC + PCoffset;
    mem_addr = (unsigned short)mem_addr;
    SR_num = StrtoBin(REGISTER[SR], 16, 1);
    BintoStr(MEMORY[mem_addr], SR_num, 16, 1);
}

void JSR(char *INS){
    unsigned short BaseR;
    short PCoffset;
    //将PC的值存入R7
    BintoStr(REGISTER[7], PC, 16, 1);
    if(INS[4] == '1'){ //JSR
        PCoffset = StrtoBin(INS + 5, 11, 1);
        PC += PCoffset;
        PC = (unsigned short)PC;
    }else{ //JSRR
        BaseR = StrtoBin(INS + 7, 3, 0);
        PC = StrtoBin(REGISTER[BaseR], 16, 1);
    }
}

void AND(char *INS){
    unsigned short DR, SR1, SR2;
    short imm5, DR_num, SR1_num, SR2_num;
    DR = StrtoBin(INS + 4, 3, 0);
    SR1 = StrtoBin(INS + 7, 3, 0);
    SR1_num = StrtoBin(REGISTER[SR1], 16, 1);
    if(INS[10] == '0'){
        SR2 = StrtoBin(INS + 13, 3, 0);
        SR2_num = StrtoBin(REGISTER[SR2], 16, 1);
    }else{
        imm5 = StrtoBin(INS + 11, 5, 1);
        SR2_num = imm5;
    }
    DR_num = SR1_num & SR2_num;
    BintoStr(REGISTER[DR], DR_num, 16, 1);
    ResetCC(DR_num);
}

void LDR(char *INS){
    unsigned short DR, BaseR, mem_addr;
    short PCoffset, DR_num, BaseR_num;
    DR = StrtoBin(INS + 4, 3, 0);
    BaseR = StrtoBin(INS + 7, 3, 0);
    PCoffset = StrtoBin(INS + 10, 6, 1);
    BaseR_num = StrtoBin(REGISTER[BaseR], 16, 0);
    mem_addr = BaseR_num + PCoffset;
    mem_addr = (unsigned short)mem_addr;
    DR_num = StrtoBin(MEMORY[mem_addr], 16, 1);
    BintoStr(REGISTER[DR], DR_num, 16, 1);
    ResetCC(DR_num);
}

void STR(char *INS){
    unsigned short SR, BaseR, mem_addr;
    short PCoffset, SR_num, BaseR_num;
    SR = StrtoBin(INS + 4, 3, 0);
    BaseR = StrtoBin(INS + 7, 3, 0);
    PCoffset = StrtoBin(INS + 10, 6, 1);
    BaseR_num = StrtoBin(REGISTER[BaseR], 16, 0);
    mem_addr = BaseR_num + PCoffset;
    mem_addr = (unsigned short)mem_addr;
    SR_num = StrtoBin(REGISTER[SR], 16, 1);
    BintoStr(MEMORY[mem_addr], SR_num, 16, 1);
}

void NOT(char *INS){
    unsigned short DR, SR;
    short DR_num, SR_num;
    DR = StrtoBin(INS + 4, 3, 0);
    SR = StrtoBin(INS + 7, 3, 0);
    SR_num = StrtoBin(REGISTER[SR], 16, 1);
    DR_num = ~SR_num;
    BintoStr(REGISTER[DR], DR_num, 16, 1);
    ResetCC(DR_num);
}

void LDI(char *INS){
    unsigned short DR, mem_addr;
    short PCoffset, DR_num;
    DR = StrtoBin(INS + 4, 3, 0);
    PCoffset = StrtoBin(INS + 7, 9, 1);
    mem_addr = PC + PCoffset;
    mem_addr = (unsigned short)mem_addr;
    mem_addr = StrtoBin(MEMORY[mem_addr], 16, 0);
    DR_num = StrtoBin(MEMORY[mem_addr], 16, 1);
    BintoStr(REGISTER[DR], DR_num, 16, 1);
    ResetCC(DR_num);
}

void STI(char *INS){
    unsigned short SR, mem_addr;
    short PCoffset, SR_num;
    SR = StrtoBin(INS + 4, 3, 0);
    PCoffset = StrtoBin(INS + 7, 9, 1);
    mem_addr = PC + PCoffset;
    mem_addr = (unsigned short)mem_addr;
    mem_addr = StrtoBin(MEMORY[mem_addr], 16, 0);
    SR_num = StrtoBin(REGISTER[SR], 16, 1);
    BintoStr(MEMORY[mem_addr], SR_num, 16, 1);
}

void JMP(char *INS){
    unsigned short BaseR;
    short PCoffset;
    BaseR = StrtoBin(INS + 7, 3, 0);
    PC = StrtoBin(REGISTER[BaseR], 16, 0);
    PC = (unsigned short)PC;
}

void LEA(char *INS){
    unsigned short DR;
    short PCoffset, DR_num;
    DR = StrtoBin(INS + 4, 3, 0);
    PCoffset = StrtoBin(INS + 7, 9, 1);
    DR_num = PC + PCoffset;
    DR_num = (unsigned short)DR_num;
    BintoStr(REGISTER[DR], DR_num, 16, 1);
}

void Decode(char *INS){
    PC++;
    unsigned short Opcode = StrtoBin(INS, 4, 0);
    switch(Opcode){
        case 0b0000: BR(INS); break;
        case 0b0001: ADD(INS); break;
        case 0b0010: LD(INS); break;
        case 0b0011: ST(INS); break;
        case 0b0100: JSR(INS); break;
        case 0b0101: AND(INS); break;
        case 0b0110: LDR(INS); break;
        case 0b0111: STR(INS); break;
        case 0b1001: NOT(INS); break;
        case 0b1010: LDI(INS); break;
        case 0b1011: STI(INS); break;
        case 0b1100: JMP(INS); break;
        case 0b1110: LEA(INS); break;
        case 0b1111: return;
        default: break;
    }
}

int main(){
    int i;
    for(i = 0; i < 0xffff; i++) strcpy(MEMORY[i], "0111011101110111");
    for(i = 0; i < 8; i++) strcpy(REGISTER[i], "0111011101110111");
    Readin();
    while(1){
        if(strcmp(MEMORY[PC], "1111000000100101") == 0) break; //HALT;
        Decode(MEMORY[PC]);
    }

    for (int i = 0; i < 8; i++) printf("R%d = x%04hX\n", i, StrtoBin(REGISTER[i],16,0));
    return 0;

}