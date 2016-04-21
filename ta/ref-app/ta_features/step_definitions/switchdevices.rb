Then(/^I switch to (.*) camera$/) do |camerafacing|
	@devices[0].backdoor("backdoorSwitchCamera", camerafacing)	
end