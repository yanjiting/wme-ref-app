step_definition_dir = "./ta_features"

f = File.new("trace/scenarios.htm", "w")

f << "<table><th>Regex</th><th>Secnarios</th><th>Tags</th><th>Source file</th>"

Dir.glob(File.join(step_definition_dir,'*.feature')).each do |step_file|
  File.new(step_file).read.each_line do |line|
    next unless line =~ /^\s*(?:Scenario)\s+/
	p line
    matches = /(?:Scenario Outline:|Scenario:)\s*(.*)/.match(line).captures
    matches << step_file
    f << "<tr>"
    f << "<td>#{matches[0]}</td>"
    f << "<td>#{matches[1]}</td>"
    f << "<td></td>"
    f << "<td><a href=\"#{matches[2]}\">#{matches[3]}</a></td>"
    f << "</tr>"
  end
end

f << "</table>"