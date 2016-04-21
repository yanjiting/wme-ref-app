Feature: Test connect to linus directly

  @test
  Scenario: As a user I can create confluence with Linus directly
    Given the Linus server is http://10.224.166.110:5000/
	When I create confluence
	Then I should get the SDP

  @test-orpheus
  Scenario: As a user I can create confluence with production Calliope
    Given I create a venue
	When I create confluence
	Then I should get the SDP
	
      @testarray
  Scenario: As a user I can create confluence with multiple sdps
    Given the Linus server is http://10.224.166.110:5000/
	When I create confluence with multiple sdps
	Then I should get the SDP
