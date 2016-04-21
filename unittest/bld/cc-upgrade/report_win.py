import re, sys, os, time
from lxml import etree

module_map = {
    "mediaengine.tp" : "tp",
    "mediaengine.util" : "util",
    "mediasession.src" : "mediasession",
    "mediaengine.wme" : "wme",
    "mediaengine.appshare" : "appshare",
    "mediaengine.shark" : "shark",
    "mediaengine.wqos" : "qos",
    "mediaengine.transmission" : "rtp",
    "mediaengine.dolphin" : "dolphin"
}

def str2bool(v):
  return v.lower() in ("yes", "true", "t", "1")
  
def parse_coverage(coverage_xml):
    root = etree.parse(coverage_xml).getroot()
    page = etree.Element('coverage')
    doc = etree.ElementTree(page)
    total_line_covered = 0
    total_line_total = 0
    total_func_covered = 0
    total_func_total = 0
    total_block_covered = 0
    total_block_total = 0
    for package in root.iter("package"):
        package_name = module_map[package.attrib["name"]]
        line_covered = 0
        line_total = 0
        func_covered = 0
        func_total = 0
        block_covered = 0
        block_total = 0
        for mthd in package.iter("method"):
            func_total += 1
            if(int(mthd.attrib["hits"]) > 0):
                func_covered += 1
        for line in package.iter("line"):
            line_total += 1
            if(int(line.attrib["hits"]) > 0):
                line_covered += 1
            if(str2bool(line.attrib["branch"])):
                branch_cov = line.attrib["condition-coverage"]
                p = re.compile('\d+')
                m = p.findall(branch_cov)
                block_total += int(m[2])
                block_covered += int(m[1])
        total_line_covered += line_covered
        total_line_total += line_total
        total_func_covered += func_covered
        total_func_total += func_total
        total_block_covered += block_covered
        total_block_total += block_total
        line_percent = "%.2f%%" % (100.0 * line_covered / line_total)
        func_percent = "%.2f%%" % (100.0 * func_covered / func_total)
        block_percent = "%.2f%%" % (100.0 * block_covered / block_total)
        etree.SubElement(page, package_name, 
                                      lines=line_percent,
                                      functions=func_percent,
                                      branches=block_percent)
    total_line_percent = "%.2f%%" % (100.0 * total_line_covered / total_line_total)
    total_func_percent = "%.2f%%" % (100.0 * total_func_covered / total_func_total)
    total_block_percent = "%.2f%%" % (100.0 * total_block_covered / total_block_total)
    page.insert(0, etree.SubElement(page, "total", 
                                      lines=total_line_percent,
                                      functions=total_func_percent,
                                      branches=total_block_percent))
    outFile = open('summary_ut.xml', 'w')
    doc.write(outFile, xml_declaration=True, encoding='utf-8', pretty_print=True)     
        
if __name__ == '__main__':
    parse_coverage("coverage.xml")