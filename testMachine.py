import subprocess
import codecs

exe_path = "./cmake-build-debug/BUAA_Compiler.exe"
# test_file_path = "D:\\Chaos\\Program\\C++\\BUAA_Compiler\\cmake-build-debug\\testfiles\\";
test_file_path = "D:\\Chaos\\课程\编译原理\\编译原理测试程序\\testfiles\\"
level = "B"
file_num = {'C': 29, 'B': 27, 'A': 26}
test_file_num = 30
_test_file_path = "./testfile.txt"
_output_file_path = "./pcoderesult.txt"

for i in range(1, file_num[level] + 1):
    # if i == 7 or i == 8 or i == 9 or i == 17 or i == 27 or i == 28:
    #     continue
    if level == 'C' and (i == 15 or i == 27 or i == 28):
        continue

    test_file_name = test_file_path + level + "\\testfile" + str(i) + ".txt"
    input_file_name = test_file_path + level + "\\input" + str(i) + ".txt"
    output_file_name = test_file_path + level + "\\output" + str(i) + ".txt"
    _test_file = open(_test_file_path, "w")

    test_file = open(test_file_name, "r")
    input_file = open(input_file_name, "r")
    print("---------------------------------------")
    print("testing: " + str(i))
    lines = test_file.readlines()
    # try:
    #
    # except UnicodeDecodeError:
    #     test_file.close()
    #     test_file = codecs.open(test_file_name, "r", 'utf-8')
    _test_file.writelines(lines)
    _test_file.close()
    test_file.close()
    subp = subprocess.Popen(exe_path, stdin=input_file)
    subp.wait(5)
    input_file.close()

    output_file = open(output_file_name, "r")
    _output_file = open(_output_file_path, "r")

    std_output = output_file.readlines()
    m_output = _output_file.readlines()
    isError = False
    for j in range(0, len(std_output)):
        l1 = std_output[j]
        l2 = m_output[j]
        # 可能出现标准输出的最后一行少一个\n
        if l1 != l2 and not l2.startswith(l1):
            print("$$$$$$$$$$$$$$$ERROR$$$$$$$$$$$$$$$$$$$$\n testfile: " + str(i) + ", in line " + str(j))
            print("std : " + l1)
            print("your: " + l2)
            print("std len: " + str(len(l1)) + ", your len: " + str(len(l2)))
            print("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$")
            isError = True
            break
    if isError is not True:
        print("AC: testfile" + str(i))


    output_file.close()
    _output_file.close()
