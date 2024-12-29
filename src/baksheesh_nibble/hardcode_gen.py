PermBits = [0, 33, 66, 99, 96, 1, 34, 67, 64, 97, 2, 35, 32, 65, 98, 3, 4, 37, 70, 103, 100, 5, 38, 71, 68, 101, 6, 39, 36, 69, 102, 7, 8, 41, 74, 107, 104, 9, 42, 75, 72, 105, 10, 43, 40, 73, 106, 11, 12, 45, 78, 111, 108, 13, 46, 79, 76, 109, 14, 47, 44, 77, 110, 15, 16, 49, 82, 115, 112, 17, 50, 83, 80, 113, 18, 51, 48, 81, 114, 19, 20, 53, 86, 119, 116, 21, 54, 87, 84, 117, 22, 55, 52, 85, 118, 23, 24, 57, 90, 123, 120, 25, 58, 91, 88, 121, 26, 59, 56, 89, 122, 27, 28, 61, 94, 127, 124, 29, 62, 95, 92, 125, 30, 63, 60, 93, 126, 31]
InvPermBits = [0, 5, 10, 15, 16, 21, 26, 31, 32, 37, 42, 47, 48, 53, 58, 63, 64, 69, 74, 79, 80, 85, 90, 95, 96, 101, 106, 111, 112, 117, 122, 127, 12, 1, 6, 11, 28, 17, 22, 27, 44, 33, 38, 43, 60, 49, 54, 59, 76, 65, 70, 75, 92, 81, 86, 91, 108, 97, 102, 107, 124, 113, 118, 123, 8, 13, 2, 7, 24, 29, 18, 23, 40, 45, 34, 39, 56, 61, 50, 55, 72, 77, 66, 71, 88, 93, 82, 87, 104, 109, 98, 103, 120, 125, 114, 119, 4, 9, 14, 3, 20, 25, 30, 19, 36, 41, 46, 35, 52, 57, 62, 51, 68, 73, 78, 67, 84, 89, 94, 83, 100, 105, 110, 99, 116, 121, 126, 115]
masks = ['0x1', '0x2', '0x4', '0x8']

TapPositions = [8, 13, 19, 35, 67, 106]

with open('baksheesh_hardcode.h', 'w') as f:
    f.write('/* This file is generated from (../src/baksheesh_nibble/hardcode_gen.py) */\n')
    f.write('#ifndef BAKSHEESH_HARDCODE_H\n\n')

    f.write('#define PBOX_HC(lhs, rhs) \\\n')
    for i in range(len(PermBits)):
        LHS_index = PermBits[i] >> 2
        RHS_index = i >> 2
        f.write(f'    lhs[{LHS_index}] |= rhs[{RHS_index}] & {masks[i & 3]};  \\\n')
    f.write('\n\n')

    f.write('#define INV_PBOX_HC(lhs, rhs) \\\n')
    for i in range(len(InvPermBits)):
        LHS_index = InvPermBits[i] >> 2
        RHS_index = i >> 2
        f.write(f'    lhs[{LHS_index}] |= rhs[{RHS_index}] & {masks[i & 3]};  \\\n')
    f.write('\n\n') 

    f.write('#define ADD_CONSTANT_HC(state, rc) \\\n')
    for i in range(len(TapPositions)):
        pos = TapPositions[i]
        idx = pos >> 2
        f.write(f'    state[{idx}] ^= (((rc >> {i}) & 0x1) << {pos & 3}); \\\n')
    f.write('\n\n')

    f.write(
"""\
#define RIGHT_SHIFT_KEY(key) \\
    do { \\
        __typeof__(key) _key = key; (void) _key; \\
        unsigned char carry = ((_key[0] & 0x1) << 3); \\
        for (int i=0; i<31; ++i) \\
            _key[i] = (_key[i] >> 1) | ((_key[i+1] & 0x1) << 3); \\
        _key[31] = (_key[31] >> 1) | carry; \\
    } while (0);
\n\n""")
    

    f.write('#endif /* BAKSHEESH_HARDCODE_H */')
    f.close()