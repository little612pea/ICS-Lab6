#encoding: utf-8
def get_ins_by_line():
    # ��ȡ�����ָ��洢���ڴ���
    addr = 0
    global pc
    pc = 0

    while True:
        try:
            line = input().strip()
            if not line:
                break
            # ʹ���б��Ƶ�ʽ���������ַ���ת��Ϊ����
            short_num = int(''.join('1' if c == '1' else '0' for c in line), 2)

            if addr == 0:
                addr = pc = short_num
            else:
                m[addr] = short_num
                addr += 1

        except EOFError:
            break

def branch(instruction):
    global pc
    pc_add9 = get_imm_offset(instruction, 9)
    condition_bits = [(instruction&0x0800, condition_codes[0]), (instruction&0x0400, condition_codes[1]), (instruction&0x0200, condition_codes[2])]
    # ʹ���б��Ƶ�ʽ�ж��Ƿ���������
    if any((bit and cond) for bit, cond in condition_bits):
        pc += pc_add9


def add(instruction):
    dest_reg = get_register_num(instruction, 9)
    s_reg_1 = get_register_num(instruction, 6)
    is_imm = instruction & 0b100000
    
    if is_imm:
        r[dest_reg] = r[s_reg_1] + get_imm_offset(instruction, 5)
    else:
        s_reg_2 = get_register_num(instruction, 0)
        r[dest_reg] = r[s_reg_1] + r[s_reg_2]
    
    set_cc_bit(dest_reg)


def load(instruction):
    dest_reg = get_register_num(instruction, 9)
    pc_add9 = get_imm_offset(instruction, 9)
    r[dest_reg] = m[(pc + pc_add9) & 0xFFFF]
    #�˴���m�ڱ������޷����������������ָ���
    #��pc+pc_add9ת��Ϊ�޷�������
    set_cc_bit(dest_reg)

def store(instruction):
    s_reg = get_register_num(instruction, 9)
    pc_add9 = get_imm_offset(instruction, 9)
    m[(pc + pc_add9) & 0xFFFF] = r[s_reg]

def jump_to_subroutine(instruction):
    global r
    global pc
    pc_add11 = get_imm_offset(instruction, 11)
    r[7] = pc
    if instruction & 0x0800:
        pc = r[get_register_num(instruction, 6)]
    else:
        pc += pc_add11

def AND(instruction):
    dest_reg = get_register_num(instruction, 9)
    s_reg_1 = get_register_num(instruction, 6)
    if instruction & 0b100000:
        r[dest_reg] = r[s_reg_1] & get_imm_offset(instruction, 5)
    else:
        s_reg_2 = get_register_num(instruction, 0)
        r[dest_reg] = r[s_reg_1] & r[s_reg_2]
    set_cc_bit(dest_reg)

def load_register(instruction):
    dest_reg = get_register_num(instruction, 9)
    BaseR = get_register_num(instruction, 6)
    pcoffset6 = get_imm_offset(instruction, 6)
    r[dest_reg] = m[(r[BaseR] + pcoffset6) & 0xFFFF]
    set_cc_bit(dest_reg)

def store_register(instruction):
    s_reg = get_register_num(instruction, 9)
    BaseR = get_register_num(instruction, 6)
    pcoffset6 = get_imm_offset(instruction, 6)
    m[(r[BaseR] + pcoffset6) & 0xFFFF] = r[s_reg]
    #ת��Ϊ�޷�������������r[BaseR] + pcoffset6 = 0b11111111111
    #

def NOT(instruction):
    dest_reg = get_register_num(instruction, 9)
    s_reg = get_register_num(instruction, 6)
    
    s_reg_value = r[s_reg]
    inverted_value = int(''.join('0' if bit == '1' else '1' for bit in bin(s_reg_value)[2:].zfill(16)), 2)
    # int
    r[dest_reg] = inverted_value & 0xFFFF  # ������ض�Ϊ16λ
    set_cc_bit(dest_reg)


def load_indirect(instruction):
    dest_reg = get_register_num(instruction, 9)
    pc_add9 = get_imm_offset(instruction, 9)
    r[dest_reg] = m[m[(pc + pc_add9) & 0xFFFF]]
    set_cc_bit(dest_reg)

def store_indirect(instruction):
    s_reg = get_register_num(instruction, 9)
    pc_add9 = get_imm_offset(instruction, 9)
    m[m[(pc + pc_add9) & 0xFFFF]] = r[s_reg]

def JMP(instruction):
    global pc
    pc = r[get_register_num(instruction, 6)]

def LEA(instruction):
    dest_reg = get_register_num(instruction, 9)
    pc_add9 = get_imm_offset(instruction, 9)
    r[dest_reg] = pc + pc_add9


def set_cc_bit(REG_DR):
    global condition_codes
    condition_codes = [0, 0, 0] 
    if r[REG_DR] < 0:
        condition_codes[0] = 1  # Set N bit
    elif r[REG_DR] == 0:
        condition_codes[1] = 1  # Set Z bit
    else:
        condition_codes[2] = 1  # Set P bit

def get_register_num(instruction, pos):
    mask = pow(2, pos + 3) - 1
    masked_value = instruction & mask
    reg_num = masked_value // (2 ** pos)
    return reg_num
    #������get_register_num(0b0101010001111011, 9)����2
    #���̣���0b0101010001111011��0b0001110000000000��λ�룬�õ�0b0001010000000000
    #�ٽ�0b0001010000000000����9λ���õ�0b0000000000000010
    #��0b0000000000000010ת��Ϊʮ���ƣ��õ�2

def get_imm_offset(instruction, LEN):
    offset = instruction & ((pow(2, LEN) - 1))
    if (offset / (pow(2, LEN - 1))) >= 1:
        # ��offsetת��Ϊ�з�������
        offset -= pow(2, LEN)
    return offset
    #������get_imm_offset(0b0101010001111011, 9)����-5
    #���̣���0b0101010001111011��0b0000001111111111��λ�룬�õ�0b0000000001111011
    #�ٽ�0b0000000001111011����9λ���õ�0b1111111111111101
    #��0b1111111111111101ת��Ϊ�з����������õ�-5


def main():
    get_ins_by_line()  
    global pc
    while True:
        instruction = m[pc]
        print(f"PC = {pc}, IR = {instruction}")
        pc += 1
        opcode = (instruction & 0xF000)//pow(2, 12)  # ��ȡ������
        if opcode == 0b1111:
            break
        # ���Ҳ������Ӧ�ĺ�����ִ��
        if opcode in opcode_to_function:
            opcode_to_function[opcode](instruction)

    for i in range(8):
        print(f"R{i} = x{r[i]:04X}")

if __name__ == "__main__":
    condition_codes = [0, 1, 0]
    r = [0x7777] * 8
    m = [0x7777] * 0xFFFF
    pc = 0
     # ���������뵽������ӳ���ֵ�
    opcode_to_function = {
        0b0000: branch, 0b0001: add,0b0010: load, 0b0011: store,
        0b0100: jump_to_subroutine,0b0101: AND,0b0110: load_register,0b0111: store_register,
        0b1001: NOT,0b1010: load_indirect,0b1011: store_indirect,0b1100: JMP,
        0b1110: LEA,
    }
    main()
