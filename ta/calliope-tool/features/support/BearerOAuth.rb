require 'uri'
require 'net/https'
require 'openssl'
#require "rubygems"
require 'json'
require 'open-uri'

class BearerOAuth
	def initialize
		@BROKER_URL = "https://idbroker.webex.com/idb";
		@BEARER_URL = @BROKER_URL + "/token/v1/actions/GetBearerToken/invoke";

		@ACCESS_URL = @BROKER_URL + "/oauth2/v1/access_token"; 
        @GENPASS_URL = @BROKER_URL + "/idbconfig/v1/actions/GeneratePassword/invoke";

		@SCOPE = "webexsquare:create_venue";
		@CLIENT_ID = "Ccebba9b7f96c8f6a0d0586359962a8e9faab518640a8de66f9e19d46453c548a";
		@CLIENT_SECRET = "19b128fe7285ec3c67daf8135ae691c991b7706b0ff00f82f7aa6bbe88fcb609";
        
		@GRANT_TYPE = "urn:ietf:params:oauth:grant-type:saml2-bearer";
		@HEALTH_CHECK_URL = @BROKER_URL + "/HealthCheck.jsp";

		@FILES_URL = "https://beta.webex.com/files/api/v1/authenticate";
		@FILES_PASSWORD = "SecretPassw0rd!";
		@FILES_EMAIL = "calliope@wx2.example.com";
		@FILES_VALID_TIME = 1 * 60 * 60 * 24 * 7;   //  #  1 week
	end
	
	def GetBearerToken
		uri = URI(@BEARER_URL)
		req = Net::HTTP::Post.new(uri.path, initheader = {'Content-Type' =>'application/json'})
		req.body = {:uid => ENV["USERID"], :password => ENV["PASSWD"]}.to_json
		
		http = Net::HTTP.new(uri.host, uri.port)
		http.use_ssl = true
		http.verify_mode = OpenSSL::SSL::VERIFY_NONE
		
		res = http.request(req)
		res_json = JSON.parse(res.body)
		return res_json["BearerToken"]
	end
	
    def GetAccessToken(bearerToken, scope=nil, clientId = nil, clientSecret=nil)
		uri = URI(@ACCESS_URL)
		req = Net::HTTP::Post.new(uri.path, 
			initheader = {
				'Content-Type' =>'application/x-www-form-urlencoded',
				'Accept' => "application/json"
			});
		req.set_form_data({:grant_type => URI::encode(@GRANT_TYPE),
						   :scope => URI::encode(scope || @SCOPE),
                           :client_id => URI::encode(clientId || @CLIENT_ID),
						   :client_secret => URI::encode(clientSecret || @CLIENT_SECRET),
						   :assertion => URI::encode(bearerToken)
						   })	
		http = Net::HTTP.new(uri.host, uri.port)
		http.use_ssl = true
		http.verify_mode = OpenSSL::SSL::VERIFY_NONE
		
		res = http.request(req)
		res_json = JSON.parse(res.body)
		"Bearer " + res_json["access_token"]
	end
	
	def GetAuthHeader
		bearerToken = GetBearerToken()
		GetAccessToken(bearerToken)
	end
end
