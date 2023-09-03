#encoding: utf-8
def AND(instruction):
    dest_reg = get_value(instruction,0, 9)
    s_reg_1 = get_value(instruction,0, 6)
    if instruction & 0b100000:
        r[dest_reg] = r[s_reg_1] & get_value(instruction,1, 5)
        print("here")
        print(get_value(instruction,1, 5))
    else:
        s_reg_2 = get_value(instruction, 0)
        r[dest_reg] = r[s_reg_1] & r[s_reg_2]

def get_value(instruction, choose, pos):
    # choose为0时返回寄存器编号，为1时返回立即数
    if choose == 0:
        mask = pow(2, pos + 3) - 1
        masked_value = instruction & mask
        reg_num = masked_value // (2 ** pos)
        return reg_num
    else:
        offset = instruction & ((pow(2, pos) - 1))
        # 进行符号扩展，将offset转换为有符号整数
        if offset & (1 << (pos - 1)):
            offset -= (1 << pos)
        #
        return offset
condition_codes = [0, 1, 0]
r = [0xFFFF] * 8
pc = 0
#add r2 r1 #-5
offset=AND(0b0101010001111011)
print(r[2])
#打印r[2]的16进制值
print(hex(r[2]))