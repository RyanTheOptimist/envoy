#import <Foundation/Foundation.h>

@interface EnvoyProxySettings : NSObject

@property (nonatomic, strong, readonly) NSString *host;
@property (nonatomic, assign, readonly) NSUInteger port;
@property (nonatomic, assign, readonly) BOOL isDirect;

- (instancetype)initWithHost:(NSString *)host port:(NSUInteger)port;

+ (instancetype)directProxy;

@end
