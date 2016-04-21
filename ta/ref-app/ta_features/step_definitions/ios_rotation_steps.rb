Then(/^I rotate (left|right|down)$/) do |dir|
    @devices[0].rotateHomeButton(dir)   
end

Then (/^I rotate (left|right|down) on device (\d+)$/) do|dir, idxstr|
    i = idxstr.to_i - 1
    @devices[i].rotateHomeButton(dir)
end

Then (/^I rotate device (\d+) (\d+) times every (\d+) seconds$/) do|idxstr, times, duration|
	i = idxstr.to_i - 1
	count = times.to_i
	while (count != 0) do
		@devices[i].rotateHomeButton("left")
		sleep duration.to_i
		@devices[i].rotateHomeButton("down")
		sleep duration.to_i
		@devices[i].rotateHomeButton("right")
		sleep duration.to_i
		@devices[i].rotateHomeButton("down")
		sleep duration.to_i
		count = count.to_i - 1
		p count
	end
end

# video_mode: landscape (width > height); portrait (width < height)
# landscape_mode = 0, portrait_mode = 1
def get_video_mode(device)
    stats = get_stats(device)
    local_w, local_h = stats["videoLocal"]["uWidth"], stats["videoLocal"]["uHeight"]

    return (local_w > local_h) ? 0 : 1
end 

Then (/^I check loopback-call (.*) result on device (\d+)$/) do |rotation, idxstr|
    cur_local_video_mode = get_video_mode(@devices[idxstr.to_i-1])

    if rotation == 'down'
        p 'current rotation status is down !!!!!!'
        assert(cur_local_video_mode == @local_video_mode, "loopback call: rotation (down) cass NON-passed.")
    else
        p 'current rotation status is left | right !!!!!'
        assert(cur_local_video_mode != @local_video_mode, "loopback call: rotation (left|right) cass NON-passed.") 
    end
end

Then (/^I record current local mode on device (\d+)$/) do |idxstr|
    @local_video_mode = get_video_mode(@devices[idxstr.to_i-1])
end 
