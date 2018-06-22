import traceback
import re

try:
    file = open("output.txt", "r")
    result = [0, 0, 0]
    graph = []
    for line in file:
        if line.startswith("Minimum timevalue"):
            s = re.findall('\d+', line)
            result[1] = float(s[0])
        elif line.startswith("Maximum timevalue"):
            s = re.findall('\d+', line)
            result[2] = float(s[0])
        elif line.startswith("Final number"):
            # print(line)
            s = re.findall('\d+', line)
            # print(s)
            result[0] = float(s[0])
        elif line.startswith("Timeslice"):
            t = re.findall('\d+', line)
            graph.append(t)

    print(result)
    for element in graph:
        print element
        print("Timeslice {} has {} spacelike faces.".format(element[0], element[1]))

except:
    traceback.print_exc()

finally:
    file.close
