begin
    require 'calabash-cucumber/wait_helpers'
    require 'calabash-cucumber/operations'
    require 'calabash-cucumber/launcher'
    require 'calabash-cucumber/calabash_steps'
rescue LoadError
    puts "You don't install calabash ios, so you cannot test iOS."
    module Calabash
        module Cucumber
            module Core
            end
            module Operations
            end
        end
    end
end

begin
    require 'calabash-android/color_helper'
    require 'calabash-android/operations'
    require 'calabash-android/calabash_steps'
    require 'calabash-android/management/app_installation'
rescue LoadError
    puts "You don't install calabash android, so you cannot test android."
end

require 'rbconfig'