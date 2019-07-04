/*
 * Copyright 2019 Google
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// TODO: Add short docs to the API
#import <FirebaseInstallations/FIRInstallationsAuthTokenResult.h>
#import <Foundation/Foundation.h>

@class FIRApp;

NS_ASSUME_NONNULL_BEGIN

FOUNDATION_EXPORT const NSNotificationName FIRInstallationIDDidChangeNotification;

typedef void (^FIRInstallationsIDHandler)(NSString *__nullable identifier,
                                          NSError *__nullable error)
    NS_SWIFT_NAME(InstallationsIDHandler);

typedef void (^FIRInstallationsTokenHandler)(
    FIRInstallationsAuthTokenResult *__nullable tokenResult, NSError *__nullable error)
    NS_SWIFT_NAME(InstallationsTokenHandler);

NS_SWIFT_NAME(Installations)
@interface FIRInstallations : NSObject

+ (nullable FIRInstallations *)installations;

+ (FIRInstallations *)installationsWithApp:(FIRApp *)application NS_SWIFT_NAME(installations(app:));

- (void)installationIDWithCompletion:(FIRInstallationsIDHandler)completion;

- (void)authTokenWithCompletion:(FIRInstallationsTokenHandler)completion;

- (void)authTokenForcingRefresh:(BOOL)forceRefresh
                     completion:(FIRInstallationsTokenHandler)completion;

- (void)deleteWithCompletion:(void (^)(NSError *__nullable))completion;

@end

NS_ASSUME_NONNULL_END