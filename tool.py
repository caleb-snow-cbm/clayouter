name = input('Give struct name: ')
types = input('Give types: ')
types = types.split(',')
members = input('Give members: ')
members = members.split(',')

builtin_types = dict()
builtin_types['float'] = 'FLOAT'
builtin_types['bool'] = 'BOOL'
builtin_types['uint8_t'] = 'INTEGRAL'
builtin_types['uint16_t'] = 'INTEGRAL'
builtin_types['uint32_t'] = 'INTEGRAL'
builtin_types['int8_t'] = 'INTEGRAL'
builtin_types['int16_t'] = 'INTEGRAL'
builtin_types['int32_t'] = 'INTEGRAL'

print('const char* _' + name + '_Members[] = {')
for member in members:
    print('    \"' + member + '\",')
print('};')
print('DEFINE_STRUCT_MEMBER_COUNT(' + name + ');')
print('static ' + name + ' _' + name + ';')
print('const size_t _' + name + '_Sizes[] = {')
for member in members:
    print('    sizeof(_' + name + '.' + member + '),')
print('};')
print('const size_t _' + name + '_Offsets[] = {')
for member in members:
    print('    offsetof(' + name + ', ' + member + '),')
print('};')
print('const member_info_t _' + name + '_Member_Info[] = {')
for type in types:
    if type in builtin_types:
        print('    { .type = TYPE_' + builtin_types[type] + ' },')
    else:
        print('    { .type = TYPE_STRUCT, .struct_info = &_' + type + '_Info },')
print('};')
print('DEFINE_STRUCT_INFO(' + name + ');')
