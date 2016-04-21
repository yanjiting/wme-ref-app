#import "./mac/share_capturer_mac.h"
#import <XCTest/XCTest.h>


@interface ShareCapturerMacTest : XCTestCase
{

    ShareCapturerMac *m_ShareCapturer;
}

@end

@implementation ShareCapturerMacTest


- (void)setUp
{
    [super setUp];
    m_ShareCapturer = [[ShareCapturerMac alloc] init];
}

- (void)tearDown
{
    [super tearDown];

    [m_ShareCapturer release];
    m_ShareCapturer = nil;
}


- (void)initBmpContext
{


}

- (void)createScreenShotForWindowsList
{
    XCTAssertTrue([m_ShareCapturer createScreenShotFor
                   :0 WindowsList:NULL] != NULL);


}

- (void)createShareImage
{

    XCTAssertTrue([m_ShareCapturer createShareImage] != NULL);

}



- (void)capture
{
    XCTAssertTrue([m_ShareCapturer capture] != NULL);

}

- (void)getFrameSize
{

    XCTAssertTrue([m_ShareCapturer getFrameSize].cx > 0);
    XCTAssertTrue([m_ShareCapturer getFrameSize].cy > 0);
}

- (void)isFilterOutApp
{

}

- (void)isCaptureApp
{

}




@end
