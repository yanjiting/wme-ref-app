#!/usr/bin/python

import commands
import os

def get_summary_for_each_module():
    print "get summary for each module:"
    file = open('./temp/summary.xml', 'w')
    file.write("<coverage>\n")
    output = commands.getoutput(" lcov -summary ./temp/wme_gcov.info").split('\n')
    print "Total:"
    for line in output:
        line = line.lstrip()
        if "Reading" == line[:7] or "Summary" == line[:7]:
            continue
        a = line.split(' ')
        if "lines......:" == a[0]:
            line_cov = a[1]
            print "  line_coverage = %s" %(line_cov)
        elif "functions..:" == a[0]:
            functions_cov = a[1]
            print "  functions_coverage = %s" %(functions_cov)
        elif "branches...:" == a[0]:
            branches_cov = a[1]
            print "  branches_coverage = %s" %(branches_cov)
    xml_line = '<total lines="%s" functions="%s" branches="%s"></total>\n' % (line_cov, functions_cov, branches_cov)
    file.write(xml_line)

    for f in os.listdir('./temp/info'):
        if f.lower().split('.')[-1] == 'info':
            module_name = f[:-5]
            print module_name, ":"
            output = commands.getoutput(" lcov -summary ./temp/info/%s" % (f)).split('\n')
            for line in output:
                line = line.lstrip()
                if "Reading" == line[:7] or "Summary" == line[:7]:
                    continue
                a = line.split(' ')
                if "lines......:" == a[0]:
                    line_cov = a[1]
                    print "  line_coverage = %s" %(line_cov)
                elif "functions..:" == a[0]:
                    functions_cov = a[1]
                    print "  functions_coverage = %s" %(functions_cov)
                elif "branches...:" == a[0]:
                    branches_cov = a[1]
                    print "  branches_coverage = %s" %(branches_cov)
            xml_line = '<%s lines="%s" functions="%s" branches="%s"></%s>\n' % (module_name, line_cov, functions_cov, branches_cov, module_name)
            file.write(xml_line)

    file.write("</coverage>")
    file.close

if __name__ == "__main__":
    get_summary_for_each_module()
