#!/usr/bin/env ruby
#
# Copyright (c) 2013 Eric Monti - Bluebox Security
#
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.


require 'bundler/setup'
require 'idevice'
require 'fileutils'

def fetch_crash_reports(device_udid, output_dir, file_name)
  fetch_crash_reports_with_dsym_dir(device_udid, output_dir, file_name, '../build/Release-iphoneos')
end

def fetch_crash_reports_with_dsym_dir(device_udid, output_dir, file_name, dsym_dir)
  FileUtils.mkdir_p output_dir
  Dir.chdir output_dir do
    output_file = "crashes.cpio"

    # copy all crashes from the device
    frc = Idevice::FileRelayClient.attach({:udid=>device_udid})
    puts "[.] Requesting crash reports"
    len = 0
    File.open(output_file, 'wb') do |outf|
       frc.request_sources("CrashReporter") do |chunk|
         outf.write(chunk)
       end
    end
    puts "[+] Wrote #{len} bytes to #{output_file}"

    # extract and rename crashes
    `cpio -i < crashes.cpio`
    # only grab the latest crash since we can't delete them
    Dir[File.expand_path('./**/LatestCrash-WebExSquared*.ips')].each do |file|
      i = 1
      outfile_path = "./#{file_name}.crash"
      while File.exists? outfile_path do
        outfile_path = "./#{file_name}_#{i}.crash"
        i += 1
      end
      File.open(outfile_path, "w") do |out_file|
        File.open(file).each_with_index do |line, index|
          # the first line is a dictionary that messes up Xcode
          out_file.puts line unless index == 0
        end
      end
      $stdout.puts "Finished copying file."

      # now symbolicate the crash
      ENV['DEVELOPER_DIR'] = '/Applications/XCode.app/Contents/Developer'
      dsym_path='#{dsym_dir}/WebExSquaredIntegrationTests.app.dSYM'
      symbolicatecrash_app = `find /Applications/Xcode.app -name symbolicatecrash -type f`.strip
      $stdout.puts "Found symbolicator at: #{symbolicatecrash_app}"

      symbolicating_cmd = "#{symbolicatecrash_app} #{outfile_path} #{dsym_path}"
      $stdout.puts symbolicating_cmd
      symbolicated_crash = `#{symbolicating_cmd}`
      $stdout.puts symbolicated_crash
      symbolicated_crash.gsub!(/.*?(?=Incident)/im, "")
      File.open(outfile_path, 'w') {|f| f.write(symbolicated_crash) }

      # clean up
      `rm -r var Library crashes.cpio`
    end
  end
end

def take_device_screenshot(device_udid, output_file)
  begin
    sshotr = Idevice::ScreenShotrClient.attach({:udid=>device_udid})
  rescue Idevice::LockdownError
    $stderr.puts "[-] Error: unable to connect to screenshotr",
    "    Hint: the DeveloperTools dmg must be mounted on device"
  end

  image = sshotr.take_screenshot
  ret = File.open(output_file, "w") {|f| f.write image }
  puts "[+] wrote #{ret} bytes to #{output_file}"
end

def restart_device(device_udid)
  Idevice::DiagnosticsRelayClient.attach({:udid=>device_udid}).restart(Idevice::DiagnosticsRelayClient::FLAG_WAIT_FOR_DISCONNECT)
end
