//
// Created by 20075 on 2024/6/19.
//

#ifndef EBLOCK_S3CLIENT_H
#define EBLOCK_S3CLIENT_H

#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/ListObjectsRequest.h>
#include <aws/s3/model/ListObjectsResult.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/common/byte_buf.h>
#include <string>

class S3Client {
private:
    Aws::SDKOptions options;
    Aws::Auth::AWSCredentials credentials;
    Aws::S3::S3Client *s3_client;
    std::string endpoint;
    std::string bucketname;
public:
    S3Client();
    ~S3Client();
    //列举objects
    void showListObjects(void);

    //上传指定名称对象的指定数据
    bool putObject(std::string name, unsigned long length, const char * buff);


    //下载指定名称对象的指定数据
    bool getObject(std::string name, unsigned long off, unsigned long length, char * buff);

};


#endif //EBLOCK_S3CLIENT_H
