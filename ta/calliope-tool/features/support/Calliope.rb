require 'securerandom'

# Backport of missing SecureRandom methods from 1.9
module SecureRandom
  class << self
    def method_missing(method_sym, *arguments, &block)
      case method_sym
      when :urlsafe_base64
        r19_urlsafe_base64(*arguments)
      when :uuid
        r19_uuid(*arguments)
      else
        super
      end
    end
    
    private
    def r19_urlsafe_base64(n=nil, padding=false)
      s = [random_bytes(n)].pack("m*")
      s.delete!("\n")
      s.tr!("+/", "-_")
      s.delete!("=") if !padding
      s
    end

    def r19_uuid
      ary = random_bytes(16).unpack("NnnnnN")
      ary[2] = (ary[2] & 0x0fff) | 0x4000
      ary[3] = (ary[3] & 0x3fff) | 0x8000
      "%08x-%04x-%04x-%04x-%04x%08x" % ary
    end
  end
end

class CalliopeClient
    def initialize(connectLinus = false)
        @auth_header = "nil"
        @connectLinus = connectLinus
        @orpheus_url = ENV["ORPHEUS_SERVER"] || "https://calliope-integration.wbx2.com:443/"
        @linus_url = ENV["LINUS_SERVER"] || "http://localhost:5001/"
        # For all of the REST calls made during this ruby run we'll use this consistent tracing id.
        # This is useful for debugging.
        @tracking_id = "WMEWX2TEST_" + SecureRandom.uuid + "_127"
        
        if(connectLinus)
            p "CUCUMBER: CalliopeClient linus Url - #{@linus_url}"
            result_json = doGet(@linus_url, "calliope/api/v1/ping", nil, false);
            unless(result_json["serviceState"].eql?("online"))
                p "CUCUMBER: createLinusConfluce. ping linus - #{result_json}"
                raise "CUCUMBER: Linus server is not in correct status, please check!"
            end
        else
            p "CUCUMBER: CalliopeClient orpheus Url - #{@orpheus_url}"
            @auth = BearerOAuth.new()
            @auth_header = @auth.GetAuthHeader()
            p "CUCUMBER: CalliopeClient. Auth - #{@auth_header}"
        end    
        p "CUCUMBER: CalliopeClient. tracking Id - #{@tracking_id}"
    end

    def doGet(url, path, body, useSSL = true)
        uri = URI(url + path)
        
        req = Net::HTTP::Get.new(uri.path, initheader = 
                {'Content-Type' =>'application/json',
                 'Authorization' => @auth_header,
                 'TrackingID' => @tracking_id
                })
        
        http = Net::HTTP.new(uri.host, uri.port)
        http.use_ssl = useSSL
        http.verify_mode = OpenSSL::SSL::VERIFY_NONE
        
        res = http.request(req)
        res_json = JSON.parse(res.body)
        req = nil
        http = nil
        return res_json
    end

    def doPost(url, path, body, useSSL = true)
        uri = URI(url + path)
        req = Net::HTTP::Post.new(uri.path, initheader = 
                {'Content-Type' =>'application/json',
                 'Authorization' => @auth_header,
                 'TrackingID' => @tracking_id
                })
        req.body = body
        
        http = Net::HTTP.new(uri.host, uri.port)
        http.use_ssl = useSSL
        http.verify_mode = OpenSSL::SSL::VERIFY_NONE
        
        res = http.request(req)
		if(res.code.to_i >= 300)
			Kernel.puts("Status Code=#{res.code}, body=#{res.body}")
		end
        res_json = JSON.parse(res.body)
        req = nil
        http = nil
        return res_json
    end

     def doPut(url, path, body, useSSL = true)
        uri = URI(url + path)
        req = Net::HTTP::Put.new(uri.path, initheader = 
                {'Content-Type' =>'application/json',
                 'Authorization' => @auth_header,
                 'TrackingID' => @tracking_id
                })
        req.body = body
        
        http = Net::HTTP.new(uri.host, uri.port)
        http.use_ssl = useSSL
        http.verify_mode = OpenSSL::SSL::VERIFY_NONE
        
        res = http.request(req)
    if(res.code.to_i >= 300)
      Kernel.puts("Status Code=#{res.code}, body=#{res.body}")
    end
        res_json = JSON.parse(res.body)
        req = nil
        http = nil
        return res_json
    end

    def doDelete(path, useSSL = true)
        uri = URI(path)
        req = Net::HTTP::Delete.new(uri.path, initheader = 
                {'Authorization' => @auth_header,
                 'TrackingID' => @tracking_id
                })

        http = Net::HTTP.new(uri.host, uri.port)
        http.use_ssl = useSSL
        http.verify_mode = OpenSSL::SSL::VERIFY_NONE

        return http.request(req)
    end

    def createVenue
        body = {
            :event_url => "127.0.0.1/wme_calliope_test", 
            :features => {
                :agent_version => "v2",
                :max_size => 2,
                :test_name => "wme_calliope_test"
            }
        }.to_json
        
        if(!@connectLinus)
            res_json = doPost(@orpheus_url, "calliope/api/v1/venues", body)
            return res_json["url"]
        end
    end

    def deleteVenue(path)
        res = doDelete(path)
        puts "delete venue: " + res.code
    end

    def createConfluence(venueUrl, sdpOffer)
        venueUri = URI(venueUrl)
        venueUriPaths = venueUri.path.split("/")
        body = {
            :venueUrl => venueUrl,
            :sdpOffer => sdpOffer,
            :correlationId => SecureRandom.uuid,
            :floorControlWebhook => "http://10.224.166.31/"
        }.to_json
        res_json = doPost(@orpheus_url, "calliope/api/v1/venues/#{venueUriPaths[-1]}/confluences", body);
        return {
            :sdpAnswer => res_json["sdpAnswer"],
            :confluenceUrl => res_json["url"],
            :requestFloorUrl => res_json["floorRequestUrl"],
            :releaseFloorUrl => res_json["floorReleaseUrl"]
        }
    end

    # Method creates a confluence on the linus specified by @linus_url
    # Parameters -
    #   venueId : represents the "conference" the confluence is in. can be any string
    #   sdpOffer: string representation of an offer sdp
    # Returns -
    #   sdpAnswer : string representation of an answer sdp generated by linus
    #   confluenceUrl: url pointing to the confluence resource created
    #
    def createLinusConfluence(venueId, sdpOffer, transcodeVideo = false)
        body_h = {
            #  BLEAIR - We don't actually need to pass in the venueUrl 
            # :venueUrl => 'http://localhost/calliope/api/v1/venues/'+venueId,
            :venue => venueId,
            #:sdp => sdpOffer,
            :correlationId => SecureRandom.uuid,
            :confluenceOptions => {
               :metrixPostfix => "test",
               :floorControlWebHook => "test",
               :featureToggles => [{:key => "calliope-media-balance-cluster", :val => 30}]
            },
            :venueOptions => {
                :forceVideoTranscode => transcodeVideo 
            }
        }
        if(sdpOffer.kind_of?(Array))
            body_h[:sdps] = sdpOffer
        else
            body_h[:sdp] = sdpOffer
        end
        body = body_h.to_json
        createVenueUrl = @linus_url + "calliope/api/v1/confluences"

        res_json = doPost(@linus_url, "calliope/api/v1/confluences", body, false);
        #p "CUCUMBER: createLinusConfluce. the confluenceUrl -"
        #p res_json['url']
		 
		 if(sdpOffer.kind_of?(Array))
			return { 
				:sdpAnswer => res_json["sdps"],
				:sdpAnswers => res_json["sdps"],
				:confluenceUrl => res_json["url"],
				:requestFloorUrl => res_json["actionsFloorRequest"],
				:releaseFloorUrl => res_json["actionsFloorRelease"]
			}
		else
        return { 
            :sdpAnswer => res_json["sdp"],
            :sdpAnswers => res_json["sdps"],
            :confluenceUrl => res_json["url"],
            :requestFloorUrl => res_json["actionsFloorRequest"],
            :releaseFloorUrl => res_json["actionsFloorRelease"]
        }
    end
    end
    
    # Method deletes a confluence
    # Parameters -
    #   confluenceUrl: url of the confluence resource to be deleted 
    #
    def deleteLinusConfluence(confluenceUrl)
        doDelete(confluenceUrl, !@connectLinus)
    end
    
    # Method update a confluence
    # Parameters -
    #   confluenceUrl: url of the confluence resource to be updated
    #   oldSdp: old sdp
    #   sdp: update sdp
    def updateLinusConfluence(confluenceUrl, sdp)
        body = {
            :sdp => sdp,
        }.to_json
        res_json = doPut(confluenceUrl, "", body, !@connectLinus)
        return res_json["sdp"]
    end
    
    def requestFloor(requestFloorUrl)
        body = {}.to_json
        doPut(requestFloorUrl,"",body)
    end
    
    def releaseFloor(releaseFloorUrl)
        body = {}.to_json
        doPut(releaseFloorUrl,"",body)
    end
end

