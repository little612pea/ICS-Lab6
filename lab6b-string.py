# encoding: utf-8
def get_ins_by_line():
    # 读取输入的指令并存储到内存中
    addr = 0
    global pc
    pc = 0

    while True:
        try:
            line = input().strip()
            if not line:
                break

            if addr == 0:
                addr = pc = line
            else:
                m[addr] = line
                addr += 1

        except EOFError:
            break


def load_effective_address(instruction):
    dest_reg = get_value(instruction, 0, 9)
    pc_add9 = get_value(instruction, 1, 9)
    r[dest_reg] = pc + pc_add9
    set_nzp_bit(dest_reg)


def set_nzp_bit(REG_DR):
    condition_codes = [0, 0, 0]
    if r[REG_DR] < 0:
        condition_codes[0] = 1  # Set N bit
    elif r[REG_DR] == 0:
        condition_codes[1] = 1  # Set Z bit
    else:
        condition_codes[2] = 1  # Set P bit


def get_value(instruction, choose, pos):
    if choose == 0:
        mask = pow(2, pos + 3) - 1
        masked_value = instruction & mask
        reg_num = masked_value // (2 ** pos)
        return reg_num
    else:
        offset = instruction & ((pow(2, pos) - 1))
        if (offset / (pow(2, pos - 1))) >= 1:
            offset -= pow(2, pos)
        return offset


def branch(instruction):
    global pc
    pc_add9 = get_value(instruction, 1, 9)
    condition_bits = [(0x0800, condition_codes[0]), (0x0400,
                                                     condition_codes[1]), (0x0200, condition_codes[2])]
    
    if any((bit & cond) for bit, cond in condition_bits):
        pc += pc_add9


def store_register(instruction):
    s_reg = get_value(instruction, 0, 9)
    BaseR = get_value(instruction, 0, 6)
    pcoffset6 = get_value(instruction, 1, 6)
    m[(r[BaseR] + pcoffset6) & 0xFFFF] = r[s_reg]


def add(instruction):
    dest_reg = get_value(instruction, 0, 9)
    s_reg_1 = get_value(instruction, 0, 6)
    is_imm = instruction & 0b100000

    if is_imm:
        r[dest_reg] = r[s_reg_1] + get_value(instruction, 1, 5)
    else:
        s_reg_2 = get_value(instruction, 0, 0)
        r[dest_reg] = r[s_reg_1] + r[s_reg_2]

    set_nzp_bit(dest_reg)


def load(instruction):
    dest_reg = get_value(instruction, 0, 9)
    pc_add9 = get_value(instruction, 1, 9)
    r[dest_reg] = int(m[(pc + pc_add9) & 0xFFFF], 2)
    set_nzp_bit(dest_reg)


def store(instruction):
    s_reg = get_value(instruction, 0, 9)
    pc_add9 = get_value(instruction, 1, 9)
    m[(pc + pc_add9) & 0xFFFF] = bin(r[s_reg])[2:].zfill(16)


def jump_to_subroutine(instruction):
    pc_add11 = get_value(instruction, 1, 11)
    r[7] = pc
    if instruction & 0x0800:
        pc = r[get_value(instruction, 0, 6)]
    else:
        pc += pc_add11


def AND(instruction):
    dest_reg = get_value(instruction, 0, 9)
    s_reg_1 = get_value(instruction, 0, 6)
    if instruction & 0b100000:
        r[dest_reg] = r[s_reg_1] & get_value(instruction, 1, 5)
    else:
        s_reg_2 = get_value(instruction, 0, 0)
        r[dest_reg] = r[s_reg_1] & r[s_reg_2]
    set_nzp_bit(dest_reg)


def load_register(instruction):
    dest_reg = get_value(instruction, 0, 9)
    BaseR = get_value(instruction, 0, 6)
    pcoffset6 = get_value(instruction, 1, 6)
    r[dest_reg] = int(m[(r[BaseR] + pcoffset6) & 0xFFFF], 2)
    set_nzp_bit(dest_reg)


def NOT(instruction):
    dest_reg = get_value(instruction, 0, 9)
    s_reg = get_value(instruction, 0, 6)

    s_reg_value = r[s_reg]
    inverted_value = int(''.join(
        '0' if bit == '1' else '1' for bit in bin(s_reg_value)[2:].zfill(16)), 2)
    
    r[dest_reg] = inverted_value & 0xFFFF
    set_nzp_bit(dest_reg)


def load_indirect(instruction):
    dest_reg = get_value(instruction, 0, 9)
    pc_add9 = get_value(instruction, 1, 9)
    r[dest_reg] = int(m[int(m[(pc + pc_add9) & 0xFFFF], 2)], 2)
    set_nzp_bit(dest_reg)


def store_indirect(instruction):
    s_reg = get_value(instruction, 0, 9)
    pc_add9 = get_value(instruction, 1, 9)
    m[int(m[(pc + pc_add9) & 0xFFFF], 2)] = bin(r[s_reg])[2:].zfill(16)


def jump(instruction):
    global pc
    pc = r[get_value(instruction, 0, 6)]


def main():
    get_ins_by_line()
    global pc
    pc = 0
    while True:
        instruction = m[pc]
        pc += 1
        ins_code = (instruction & 0xF000)//pow(2, 12)  # 提取操作码
        if ins_code == 0b1111:
            break
        # 查找操作码对应的函数并执行
        if ins_code in opcode_to_function:
            opcode_to_function[ins_code](instruction)

    for i in range(8):
        print(f"R{i} = x{r[i]:04X}")


if __name__ == "__main__":
    condition_codes = [0, 1, 0]
    r = [0x7777] * 8
    m = [0x7777] * 0xFFFF
    pc = 0
    # 创建操作码到函数的映射字典
    opcode_to_function = {
        0b0000: branch, 0b0001: add, 0b0010: load, 0b0011: store,
        0b0100: jump_to_subroutine, 0b0101: AND, 0b0110: load_register,
        0b0111: store_register, 0b1001: NOT, 0b1010: load_indirect,
        0b1011: store_indirect, 0b1100: jump, 0b1110: load_effective_address,
    }
    main()
