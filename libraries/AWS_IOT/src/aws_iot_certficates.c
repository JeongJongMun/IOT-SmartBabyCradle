/*
 * Copyright 2010-2015 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * Additions Copyright 2016 Espressif Systems (Shanghai) PTE LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

/**
 * @file aws_iot_certifcates.c
 * @brief File to store the AWS certificates in the form of arrays
 */

#ifdef __cplusplus
extern "C" {
#endif

const char aws_root_ca_pem[] = {"-----BEGIN CERTIFICATE-----\n\
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n\
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n\
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n\
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n\
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n\
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n\
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n\
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n\
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n\
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n\
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n\
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n\
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n\
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n\
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n\
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n\
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n\
rqXRfboQnoZsG4q5WTP468SQvvG5\n\
-----END CERTIFICATE-----\n"};

const char certificate_pem_crt[] = {"-----BEGIN CERTIFICATE-----\n\
MIIDWTCCAkGgAwIBAgIUNVkmAJpgA6rseRDd+YF4lzH+ZKgwDQYJKoZIhvcNAQEL\n\
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g\n\
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTIzMTIwMTAzMTE1\n\
OVoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0\n\
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALp4b4rJGBYugbNCp57V\n\
LE3KNvFs2gpG8nYQfqV8a6TXg+TFWASQCywzlXu0UugrrjfOYEHExnyoovOoojcq\n\
s18OAVPpDVEjVXZavhCj79zGhz6zz2tR4Si0I2BUuFup0lxMIhr4tq0z9K7m3qW2\n\
j99jv2n4Hvqu9BlkDSZlRjGL87WwQrp422hKUmFDC7C+0w2F2w+RU96/bywGIT0r\n\
dCNCYGoMYdsMXiU23GS/Iv7ECS0+A3NlgLXGkMsrlra2Rz2bceq2NHYSAriIuNgz\n\
D2VQEd9eUhM+rYjt4xveJcl+P7kL3hEA6rQHVs5HeeKs9vQYXX957Urf+PCNvffE\n\
B5MCAwEAAaNgMF4wHwYDVR0jBBgwFoAUbhoZ6xzfJACzVpdgVKXUU08bnAYwHQYD\n\
VR0OBBYEFOTmUvPCkAAeV9JAnwMI7crIwYpeMAwGA1UdEwEB/wQCMAAwDgYDVR0P\n\
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQBWwBb/4V5kDx3wX+EMN0x8/sqP\n\
JztU2rsY5lSniqSm+5U8aQ3/nMyNGdbh13flDSsUzubJ9WLYG8tPOxF4M7/E7TC5\n\
bb+yAO61ciS+MeY5+zf4BrX7ddc1vEtqW2lIBrqQlGdFMcJ7t1xkg5FYCFBUjNjw\n\
/84PPCIzmQPusPDYN32ElwdxY1vXSpSKzPeNYF+kvsdu7tQmT/CvG09ipsq9g7Wc\n\
2S6ccxfOH7113dcJHmBWFbMj7RL4FdRbWIznSYMWDBJkADjssMDdTUnHUlKwvBYH\n\
WMtn1YslP1RL+5TUHexEfJCAtQtIoiOPACgj6OMB2BKd2fw5vh4Xccx2lYzh\n\
-----END CERTIFICATE-----\n"};

const char private_pem_key[] = {"-----BEGIN RSA PRIVATE KEY-----\n\
MIIEpAIBAAKCAQEAunhviskYFi6Bs0KnntUsTco28WzaCkbydhB+pXxrpNeD5MVY\n\
BJALLDOVe7RS6CuuN85gQcTGfKii86iiNyqzXw4BU+kNUSNVdlq+EKPv3MaHPrPP\n\
a1HhKLQjYFS4W6nSXEwiGvi2rTP0rubepbaP32O/afge+q70GWQNJmVGMYvztbBC\n\
unjbaEpSYUMLsL7TDYXbD5FT3r9vLAYhPSt0I0Jgagxh2wxeJTbcZL8i/sQJLT4D\n\
c2WAtcaQyyuWtrZHPZtx6rY0dhICuIi42DMPZVAR315SEz6tiO3jG94lyX4/uQve\n\
EQDqtAdWzkd54qz29Bhdf3ntSt/48I2998QHkwIDAQABAoIBAA5dovJFRdbHpHRk\n\
CTfiYnRnxM3avf2Ws61G3grvx/c+J0ojrgFD+6hfHQAXx19dpiFkg+PtPgFymIM/\n\
Fag8YO9My+tkBAOhPj9NBGGRz7Ppk5fQEIXLHBafsi4EuONO9QbpUqNTT/eODia/\n\
YJ3Rr8ndDG0kkXZsrh+apqyzlrHQXYvc3EYim16OcZn6zI09eryoIyEuuZ0FKXFa\n\
JAiFXTkrTY78BcTef5GNsRtRhkOUz6BHYLX9RDI0XR+6U9ur+6hBYni+YoCGIwu9\n\
pSyGX85PGkYzCQuA+FTptKHPbuT+XbQVQNyT6N4GvBwpfPYJjEdV4Wy82LCvy9O+\n\
6q2TUykCgYEA85wRP2K4VXdbQcR9kj8eA8NuL9ZROjJowMdIo3MhK+4A0xN9pp1B\n\
6JSwE+KMfimRBJ0o04a5MoB6bN6bANip1xb0QQ/3cdUswp5OqlGpd7vqWmZ+42/k\n\
gaUFaVh2phQlMfhED9Zs3CnOwIXaWzyTnwmvxzAs3MVW7hzja77gxZ8CgYEAw/Ri\n\
anke6CUQ7TM852q7IDRl3SZlbZmKBR7Xe4ZU1B8fcVXecv6z38jyI5IjJl+3ZKQC\n\
RoT8gieuO/Pk0wPujqmEBEepp8KunUARJMaMNlPqoXOieYMNuDETi6+gdBNjkItX\n\
c0aoQ8gMXfT9Qy3bKlbntkWnWFdE+QCm5w/EcY0CgYAUU0UtILN4jgjTJ+3CMfnz\n\
Hm8nFp6YWHRdAUOSP5zY6XawJcQqDpzdJuq5/gDy1cz/sY0ieO8iw41zX+9W1rz0\n\
8TlX5Fkk6kBdzI+LrlyRXOBkwoB7wW4zUu1sqLDdkni3/prPJ3jVtbdGYOSjEB7Q\n\
+iHsYJydnult3IMtDjP6yQKBgQCSiH3QASMLlixCNL6mjzm/q29BISVYACkmCxo4\n\
VQu6VEu/BOeENtNr1Wi9ENsYJxuNNtg8w9XPvrycOJY09gu+wyTlznEf8oWBMhef\n\
2JmhRr9+XGpzU30+7/Qd0Qe7xYP+4zRSD9RkW4Rj/oAlbHUoBDTiKNQrS5dYXRRy\n\
VWghQQKBgQDfjTjJ/YfNAFVBgqHOBJ3SQz7HwsPdAyciwOt3JpxBNU5tMUY5fyiH\n\
/BoZ36yIxwJbnbNcHq5lcfkUuQnzvjiC2q1lAIkzFnys2CTtdnFKzK/c8hEBWPP1\n\
Tf0cMKBT+6gbsNICJfsgDB+AzlPlz5940nt+IaTrJ5QMyezdiGyFEA==\n\
-----END RSA PRIVATE KEY-----\n"};


#ifdef __cplusplus
}
#endif
