f = File.new("output.htm", "w+")
f << "<table><th>File</th><th>Tags</th><th>Description</th>"
Dir.glob(File.join('../ta_features','**/*.feature')).each do |feature_file|
	tags_line = ""
	File.new(feature_file).read.each_line do |line|
		unless line =~ /^\s*(Scenario).*/
			tags_line = line.strip
			next 
		end	
		f << "<tr>"
		f << "<td>#{feature_file}</td>"
		f << "<td>#{tags_line}</td>"
		f << "<td>#{line.strip}</td>"
		f << "</tr>"
	end	
end
f << "</table>"
