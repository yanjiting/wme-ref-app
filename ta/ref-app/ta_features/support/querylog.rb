begin

require 'calabash-cucumber/core'
require 'calabash-cucumber/tests_helpers'
require 'calabash-cucumber/keyboard_helpers'
require 'calabash-cucumber/wait_helpers'
require 'calabash-cucumber/version'
#require 'calabash-cucumber/location'

require 'net/http'
require 'test/unit/assertions'
require 'json'
require 'set'

module Calabash
    module Cucumber
        module Operations
            include Test::Unit::Assertions
            include Calabash::Cucumber::Core
            include Calabash::Cucumber::TestsHelpers
            include Calabash::Cucumber::WaitHelpers
            include Calabash::Cucumber::KeyboardHelpers
            #include Calabash::Cucumber::Location
            
            def query_log(args)
                res = http({:method => :get, :raw => true, :path => 'querylog'}, args)
                res = JSON.parse(res)
                if (res['outcome'] != 'SUCCESS')
                    screenshot_and_raise "get query_log failed because: #{res['reason']}\n#{res['details']}"
                    puts("query_log failed")
                end
                
                res['results']
            end
            
        end
    end
end

rescue LoadError
end
