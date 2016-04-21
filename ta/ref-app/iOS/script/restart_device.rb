#!/usr/bin/env ruby

# Usage:
#
# restart_device device_udid
#

require_relative './idevicehelpers.rb'

device_udids = ARGV
if device_udids.size == 0
  $stdout.puts "No devices provided, restarting all attached devices."
  device_udids = Idevice.device_list
end

device_udids.each do |device_udid|
  $stdout.puts "Restarting Device UDID: #{device_udid}"
  restart_device device_udid
end

exit 0
