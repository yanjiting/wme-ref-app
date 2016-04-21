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
    require 'calabash-android/java_keystore'
    require 'calabash-android/helpers'
    require 'calabash-android/env'
rescue LoadError
    module Calabash
    module Android
    module Operations
    end
    end
    end
    puts "You don't install calabash android, so you cannot test android."
end

begin
    require File.join(File.dirname(__FILE__), '../../../calliope-tool/features/support', 'Calliope')
    require File.join(File.dirname(__FILE__), '../../../calliope-tool/features/support', 'BearerOAuth')
rescue LoadError
    puts "You don't have calliope API implementations."
end
 
require 'rbconfig'

class String
  def to_bool
    return true if self == true || self =~ (/^(true|t|yes|y|1)$/i)
    return false if self == false || self =~ (/^(false|f|no|n|0)$/i)
    raise ArgumentError.new("invalid value for Boolean: \"#{self}\"")
  end
  
  def is_bool?
	return true if self == true || self =~ (/^(true|yes)$/i)
    return true if self == false || self =~ (/^(false|no)$/i)
	return false
  end
end

class FalseClass
  def to_bool
    return self
  end
end

class TrueClass
  def to_bool
    return self
  end
end
