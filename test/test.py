from bindings import *
from models import models

test_data = bytes(b & 0xff for b in range(256))
failed = False

def check(test_name, test_value, actual_value, print_result_if_true=True):
    result = test_value == actual_value

    if (result and print_result_if_true) or not result:
        print(f'{test_name + ':':<11} {test_value:#x} {actual_value:#x} {result}')

    if not result:
        global failed
        failed = True

for name, model in models.items():
    print(name)
    params = crc_params(*model)

    #print braid table
    """
    table = list(list(arr) for arr in params.braid_table)
    for t in table:
        for e in t:
            print(hex(e), end=' ')
        print('\n')
    """

    #test crc_table
    value = crc_table(params, params.init, b'123456789')
    check('Table', value, model.check)

    #len < 24
    value = crc_braid(params, params.init, test_data[:10])
    value2 = crc_table(params, params.init, test_data[:10])
    check('len < 24', value, value2)

    #len > 24
    value = crc_braid(params, params.init, test_data)
    value2 = crc_table(params, params.init, test_data)
    check('len > 24', value, value2)

    #chunked
    value = crc_braid(params, params.init, test_data[:100])
    value = crc_braid(params, value, test_data[100:])
    value2 = crc_table(params, params.init, test_data)
    check('Chunked', value, value2)

    #unaligned
    for i in range(1, 16):
        value = crc_braid_unaligned(params, params.init, test_data, i)
        value2 = crc_table(params, params.init, test_data[i:])
        check('Unaligned', value, value2, False)

    print()

if failed:
    raise Exception('Test failed')
else:
    print('The test ran successfully!')