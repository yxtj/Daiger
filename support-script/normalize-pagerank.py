import os, sys

def loadData(fn):
	f=open(fn)
	res={}
	s=0.0
	for line in f:
		t=line.split('\t')
		key=int(t[0])
		value=float(t[1])
		s+=value
		res[key]=value
	return res,s


def dumpData(data, opref, n):
    # open files
    fns = []
    for i in range(n):
        f = open(opref+str(i), 'w')
        fns.append(f)
    # dump
    for k in data:
        fns[k%n].write("%d\t%f\n" % (k, data[k]))
    # close files
    for i in range(n):
        fns[i].close()


def main(n, ipref, opref):
    data = {}
    sum = 0.0
    for i in range(n):
        fn = ipref + str(i)
        print('  loading file: '+fn)
        temp, s = loadData(fn)
        data.update(temp)
        sum += s
    print('sum is', sum)
    for k in data:
        data[k] /= sum
    if opref == "":
        for k in data:
            print("%d\t%f" % (k, data[k]))
    else:
        dumpData(data, opref, n)
        

if __name__=='__main__':
    argc = len(sys.argv)
    if argc <= 2:
        print('Normalize the result of PageRank.')
        print('Usage: <#parts> <in-prefix> [out-prefix]')
        print('  <#parts>: the number of parts of the input data.')
        print('  <in-prefix>: the prefix of input file(s). The actual file name(s) are <in-prefix><part-id>. <part-id> starts from 0.')
        print('  <out-prefix>: (default: <blank>) the prefix of output file(s). If left blank, output to console. Otherwise, output to files (assume folder exists). It is safe to overwrite the input(s). The actual file name(s) are <out-name-prefix><part-id>.')
        exit()
    n_parts=int(sys.argv[1])
    in_prefix=sys.argv[2]
    out_prefix=""
    if argc > 3 and sys.argv[3] not in ["", "-"] :
        out_prefix=sys.argv[3]
    main(n_parts, in_prefix, out_prefix)

