#!/usr/bin/env python

# Copyright 2011-2012 Eric Wendelin
#
# This is free software, licensed under the Apache License, Version 2.0,
# available in the accompanying LICENSE.txt file.

"""
Converts visual studio line coverage output to Cobertura-compatible XML for CI
"""

import re, sys, os, time
from xml.dom import minidom
from lxml import etree
from optparse import OptionParser

VERSION = '1.4'
__all__ = ['VSCobertura']

class VSCobertura(object):
    """
    Converts code coverage report files in lcov format to Cobertura's XML
    report format so that CI servers like Jenkins can aggregate results and
    determine build stability etc.

    >>> from lcov_cobertura import LcovCobertura
    >>> LCOV_INPUT = 'your lcov input'
    >>> converter = LcovCobertura(LCOV_INPUT)
    >>> cobertura_xml = converter.convert()
    >>> print cobertura_xml
    """
    
    def __init__(self, base_dir = "."):
        self.base_dir = base_dir
        self.coverage_data = {
            'packages': {},
            'summary': {'lines-total': 0, 'lines-covered': 0,
                        'branches-total': 0, 'branches-covered': 0},
            'timestamp': str(int(time.time()))
        }
        pass
    
    def getText(self, dom, tagName):
        return dom.findtext(tagName)
    
    def addCoverage(self, xml_coverage, filefilter=None, excludes=None):
        """
        Create a new :class:`LcovCobertura` object using the given `lcov_data`
        and `options`.

        :param xml_coverage: Path to LCOV data file
        :type xml_coverage: string
        :param filefilter: list of regexes to files as included
        :type filefilter: string
        :param excludes: list of regexes to files as excluded
        :type excludes: [string]
        """

        starttime = int(round(time.time() * 1000))
        dom_data = etree.parse(xml_coverage).getroot()
        print(int(round(time.time() * 1000)) - starttime)
        outFiles = []
        for fileIt in dom_data.iter("SourceFileNames"):
            filename = fileIt.findtext("SourceFileName")
            #print(filename)
            notfound = True
            for ftIt in filefilter:
                if ftIt in filename:
                    notfound = False
                    break
            if(notfound):
                continue
                
            for exclude in excludes:
                if exclude in filename:
                    notfound = True
                    break
            if(not notfound):
                fileid = fileIt.findtext("SourceFileID")
                outFiles.append((filename, fileid))
        #print(outFiles)
        print(int(round(time.time() * 1000)) - starttime)
        
        for fileInfo in outFiles:
            file_name = fileInfo[0]
            file_id = fileInfo[1]
            relative_file_name = os.path.relpath(file_name, self.base_dir)
            package = '.'.join(relative_file_name.split(os.path.sep)[0:2])
            #print(package)
            class_name = file_name.split(os.path.sep)[-1]
            if package not in self.coverage_data['packages']:
                self.coverage_data['packages'][package] = {
                    'classes': {}, 'lines-total': 0, 'lines-covered': 0,
                    'branches-total': 0, 'branches-covered': 0
                }
            self.coverage_data['packages'][package]['classes'][relative_file_name] = {
                'name': class_name, 'lines': {}, 'lines-total': 0,
                'lines-covered': 0, 'branches-total': 0,
                'branches-covered': 0
            }
            file_lines_total = 0
            file_lines_covered = 0
            file_lines = {}
            file_methods = {}
            file_branches_total = 0
            file_branches_covered = 0
            
            for method in dom_data.iter("Method"):
                for line in method.iter("Lines"):
                    method_fileid = line.findtext("SourceFileID")
                    break
                if(file_id == method_fileid):
                    method_name = self.getText(method, "MethodName")
                    lines_covered = int(self.getText(method, "LinesCovered"))
                    partial_covered = int(self.getText(method, "LinesPartiallyCovered"))
                    lines_notcovered = int(self.getText(method, "LinesNotCovered"))
                    blocks_covered = int(self.getText(method, "BlocksCovered"))
                    blocks_notcovered = int(self.getText(method, "BlocksNotCovered"))
                    if(lines_covered > 0):
                        file_methods[method_name] = "1"
                    else :
                        file_methods[method_name] = "0"
                    file_lines_covered = file_lines_covered + lines_covered + partial_covered
                    file_lines_total = file_lines_total + lines_covered + partial_covered + lines_notcovered
                    file_branches_total = file_branches_total + blocks_notcovered + blocks_covered
                    file_branches_covered = file_branches_covered + blocks_covered
                    
                    first_line = 0
                    tmp_block_total = blocks_notcovered + blocks_covered
                    tmp_block_covered = blocks_covered
                    for line in method.iter("Lines"):
                        coverage = int(self.getText(line, "Coverage"))
                        line_start = int(self.getText(line, "LnStart"))
                        line_end = int(self.getText(line, "LnEnd"))
                        #print("-- %d~%d" % (line_start, line_end))
                        for line_number in range(line_start, line_end + 1):
                            if(first_line == 0):
                                first_line = line_number
                            if line_number not in file_lines:
                                file_lines[line_number] = {
                                    'branch': 'false', 'branches-total': 0,
                                    'branches-covered': 0
                                }
                            if(coverage == 2):
                                if('hits' not in file_lines[line_number]):
                                    file_lines[line_number]['hits'] = 0
                            else:    
                                if('hits' not in file_lines[line_number]):
                                    file_lines[line_number]['hits'] = 1
                                else:    
                                    file_lines[line_number]['hits'] += 1
                                    
                            if(coverage == 1):
                                file_lines[line_number]['branch'] = 'true'
                                file_lines[line_number]['branches-covered'] = 1
                                file_lines[line_number]['branches-total'] = 2
                                tmp_block_total -= 2
                                tmp_block_covered -= 1
                    if(tmp_block_total > 0 and tmp_block_covered > 0):
                        file_lines[first_line]['branch'] = 'true'            
                        file_lines[first_line]['branches-covered'] = tmp_block_covered
                        file_lines[first_line]['branches-total'] = tmp_block_total
                    #break            
            package_dict = self.coverage_data['packages'][package]
            package_dict['lines-total'] += file_lines_total
            package_dict['lines-covered'] += file_lines_covered
            package_dict['branches-total'] += file_branches_total
            package_dict['branches-covered'] += file_branches_covered
            file_dict = package_dict['classes'][relative_file_name]
            file_dict['lines-total'] = file_lines_total
            file_dict['lines-covered'] = file_lines_covered
            file_dict['lines'] = dict(file_lines)
            file_dict['methods'] = dict(file_methods)
            file_dict['branches-total'] = file_branches_total
            file_dict['branches-covered'] = file_branches_covered
            self.coverage_data['summary']['lines-total'] += file_lines_total
            self.coverage_data['summary']['lines-covered'] += file_lines_covered
            self.coverage_data['summary']['branches-total'] += file_branches_total
            self.coverage_data['summary']['branches-covered'] += file_branches_covered

            #break
        #print(self.coverage_data)    
        print(int(round(time.time() * 1000)) - starttime)

    def convert(self):
        """
        Convert lcov file to cobertura XML using options from this instance.
        """
        self.calculate_rate()
        return self.generate_cobertura_xml(self.coverage_data)

    def calculate_rate(self):
        # Compute line coverage rates
        for package_data in list(self.coverage_data['packages'].values()):
            package_data['line-rate'] = self._percent(
                package_data['lines-total'],
                package_data['lines-covered'])
            package_data['branch-rate'] = self._percent(
                package_data['branches-total'],
                package_data['branches-covered'])

    def generate_cobertura_xml(self, coverage_data):
        """
        Given parsed coverage data, return a String cobertura XML representation.

        :param coverage_data: Nested dict representing coverage information.
        :type coverage_data: dict
        """

        dom_impl = minidom.getDOMImplementation()
        doctype = dom_impl.createDocumentType("coverage", None,
                                              "http://cobertura.sourceforge.net/xml/coverage-03.dtd")
        document = dom_impl.createDocument(None, "coverage", doctype)
        root = document.documentElement
        summary = coverage_data['summary']
        self._attrs(root, {
            'branch-rate': self._percent(summary['branches-total'],
                                         summary['branches-covered']),
            'branches-covered': str(summary['branches-covered']),
            'branches-valid': str(summary['branches-total']),
            'complexity': '0',
            'line-rate': self._percent(summary['lines-total'],
                                       summary['lines-covered']),
            'lines-valid': str(summary['lines-total']),
            'timestamp': coverage_data['timestamp'],
            'version': '1.9'
        })

        sources = self._el(document, 'sources', {})
        source = self._el(document, 'source', {})
        source.appendChild(document.createTextNode(self.base_dir))
        sources.appendChild(source)

        root.appendChild(sources)

        packages_el = self._el(document, 'packages', {})

        packages = coverage_data['packages']
        for package_name, package_data in list(packages.items()):
            package_el = self._el(document, 'package', {
                'line-rate': package_data['line-rate'],
                'branch-rate': package_data['branch-rate'],
                'name': package_name
            })
            classes_el = self._el(document, 'classes', {})
            for class_name, class_data in list(package_data['classes'].items()):
                class_el = self._el(document, 'class', {
                    'branch-rate': self._percent(class_data['branches-total'],
                                                 class_data['branches-covered']),
                    'complexity': '0',
                    'filename': class_name,
                    'line-rate': self._percent(class_data['lines-total'],
                                               class_data['lines-covered']),
                    'name': class_data['name']
                })

                # Process methods
                methods_el = self._el(document, 'methods', {})
                for method_name, hits in list(class_data['methods'].items()):
                    method_el = self._el(document, 'method', {
                        'name': method_name,
                        'signature' : '',
                        'hits': hits
                    })
                    methods_el.appendChild(method_el)

                # Process lines
                lines_el = self._el(document, 'lines', {})
                lines = list(class_data['lines'].keys())
                lines.sort()
                for line_number in lines:
                    line_el = self._el(document, 'line', {
                        'branch': class_data['lines'][line_number]['branch'],
                        'hits': str(class_data['lines'][line_number]['hits']),
                        'number': str(line_number)
                    })
                    if class_data['lines'][line_number]['branch'] == 'true':
                        total = int(class_data['lines'][line_number]['branches-total'])
                        covered = int(class_data['lines'][line_number]['branches-covered'])
                        percentage = int((covered * 100.0) / total)
                        line_el.setAttribute('condition-coverage',
                                             '{0}% ({1}/{2})'.format(
                                                 percentage, covered, total))
                    lines_el.appendChild(line_el)

                class_el.appendChild(methods_el)
                class_el.appendChild(lines_el)
                classes_el.appendChild(class_el)
            package_el.appendChild(classes_el)
            packages_el.appendChild(package_el)
        root.appendChild(packages_el)

        return document.toprettyxml()

    def _el(self, document, name, attrs):
        """
        Create an element within document with given name and attributes.

        :param document: Document element
        :type document: Document
        :param name: Element name
        :type name: string
        :param attrs: Attributes for element
        :type attrs: dict
        """
        return self._attrs(document.createElement(name), attrs)

    def _attrs(self, element, attrs):
        """
        Set attributes on given element.

        :param element: DOM Element
        :type element: Element
        :param attrs: Attributes for element
        :type attrs: dict
        """
        for attr, val in list(attrs.items()):
            element.setAttribute(attr, val)
        return element

    def _percent(self, lines_total, lines_covered):
        """
        Get the percentage of lines covered in the total, with formatting.

        :param lines_total: Total number of lines in given module
        :type lines_total: number
        :param lines_covered: Number of lines covered by tests in module
        :type lines_covered: number
        """

        if lines_total == 0:
            return '0.0'
        return str(float(float(lines_covered) / float(lines_total)))

if __name__ == '__main__':
    def main(argv):
        """
        Converts LCOV coverage data to Cobertura-compatible XML for reporting.

        Usage:
            lcov_cobertura.py lcov-file.dat
            lcov_cobertura.py lcov-file.dat -b src/dir -e test.lib -o path/out.xml

        By default, XML output will be written to ./coverage.xml
        """

        parser = OptionParser()
        parser.usage = 'lcov_cobertura.py lcov-file.dat [-b source/dir] [-e <exclude packages regex>] [-o output.xml]'
        parser.description = 'Converts lcov output to cobertura-compatible XML'
        parser.add_option('-b', '--base-dir', action='store',
                          help='Directory where source files are located',
                          dest='base_dir', default='.')
        parser.add_option('-o', '--output',
                          help='Path to store cobertura xml file',
                          action='store', dest='output', default='coverage.xml')
        (options, args) = parser.parse_args(args=argv)

        if len(args) != 2:
            print((main.__doc__))
            sys.exit(1)

        try:
            vs_cobertura = VSCobertura("c:\\myworkspace\\wx2\\latest\\wme\\")
            vs_cobertura.addCoverage(args[1], ["wme\\mediasession", "wme\\mediaengine"], 
                                     ["include", "unittest"])
            cobertura_xml = vs_cobertura.convert()
            with open(options.output, mode='wt') as output_file:
                output_file.write(cobertura_xml)
        except IOError:
            sys.stderr.write("Unable to convert %s to Cobertura XML" % args[1])

    main(sys.argv)
