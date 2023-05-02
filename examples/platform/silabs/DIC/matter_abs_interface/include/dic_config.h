/**
 * @file
 * @brief Matter abstraction layer for Direct Internet Connectivity.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc.
 *www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon
 *Laboratories Inc. Your use of this software is
 *governed by the terms of Silicon Labs Master
 *Software License Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.
 *This software is distributed to you in Source Code
 *format and is governed by the sections of the MSLA
 *applicable to Source Code.
 *
 ******************************************************************************/

#ifndef __DIC_CONFIG_H
#define __DIC_CONFIG_H

/*certificates*/
#define USE_AWS 1
#define USE_MOSQUITTO 0

/* Task Configuration*/
#define DIC_TASK_NAME "DIC"
#define DIC_TASK_STACK_SIZE (2*1024)  //2k

/* Network Configuration*/
#define DIC_SERVER_HOST	"a2m21kovu9tcsh-ats.iot.ap-southeast-1.amazonaws.com"
#define DIC_SERVER_PORT	8883
#define DIC_SERVER_CA_CERT  ca_certificate
#define DIC_DEVICE_CERT     device_certificate
#define DIC_DEVICE_KEY      device_key

#define DIC_KEEP_ALIVE 0

#if USE_AWS

#define DIC_CLIENT_ID "DIC_2"
#define DIC_CLIENT_USER NULL
#define DIC_CLIENT_PASS NULL

char ca_certificate[] = "-----BEGIN CERTIFICATE-----\n"
"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n"
"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n"
"b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n"
"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n"
"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n"
"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n"
"9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n"
"IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n"
"VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n"
"93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n"
"jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n"
"AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n"
"A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n"
"U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n"
"N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n"
"o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n"
"5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n"
"rqXRfboQnoZsG4q5WTP468SQvvG5\n"
"-----END CERTIFICATE-----";

char device_certificate[] = "-----BEGIN CERTIFICATE-----\n"
"MIIB/DCCAaMCFFvLxRPNwTeVQB5YtKLbEdwdcvMaMAoGCCqGSM49BAMCMIGAMQsw\n"
"CQYDVQQGEwJJTjESMBAGA1UECAwJVGVsYW5nYW5hMQwwCgYDVQQHDANIeWQxDzAN\n"
"BgNVBAoMBlNpbGFiczEPMA0GA1UECwwGbWF0dGVyMQ8wDQYDVQQDDAZjaGFuZHUx\n"
"HDAaBgkqhkiG9w0BCQEWDXNoQHNpbGFicy5jb20wHhcNMjMwMzI0MTIwMzI5WhcN\n"
"MjQwMzE4MTIwMzI5WjCBgDELMAkGA1UEBhMCSU4xEjAQBgNVBAgMCVRlbGFuZ2Fu\n"
"YTEMMAoGA1UEBwwDSHlkMQ8wDQYDVQQKDAZTaWxhYnMxDzANBgNVBAsMBm1hdHRl\n"
"cjEPMA0GA1UEAwwGY2hhbmR1MRwwGgYJKoZIhvcNAQkBFg1zaEBzaWxhYnMuY29t\n"
"MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEWIaIixoLW8k3JFROV0Uy+ja2Awhv\n"
"TnYXuuogAqbMsCsNB2kix3zIU6d8uYM/0QjVGBo3NT5S32BlvKa1aHaD+TAKBggq\n"
"hkjOPQQDAgNHADBEAiAMm4+cD3en9qYFHnRpl0YmVlj2bWpJ+VSDRlkUru2JdgIg\n"
"CLtDL0HQ9NZt+AwKGYwnBZwJLOMuBWczU4uCcXsFGUM=\n"
"-----END CERTIFICATE-----";

char device_key[] = "------BEGIN EC PRIVATE KEY-----\n"
"MHcCAQEEIFuiGwgW2Xj8QC+SO9LXNf9v6aSb+1vqAstp2h+dUArzoAoGCCqGSM49\n"
"AwEHoUQDQgAEWIaIixoLW8k3JFROV0Uy+ja2AwhvTnYXuuogAqbMsCsNB2kix3zI\n"
"U6d8uYM/0QjVGBo3NT5S32BlvKa1aHaD+Q==\n"
"-----END EC PRIVATE KEY-----";

#elif USE_MOSQUITTO

#define DIC_CLIENT_ID "test_client"
#define DIC_CLIENT_USER "explorer1"
#define DIC_CLIENT_PASS "p@ssw0rd"

char ca_certificate[] = "-----BEGIN CERTIFICATE-----\n"
"MIIDizCCAnOgAwIBAgIUHSLE8e28d9aCpdIyjFsL4hBhRuswDQYJKoZIhvcNAQEL\n"
"BQAwVTELMAkGA1UEBhMCSU4xCzAJBgNVBAgMAktBMQswCQYDVQQHDAJCQTELMAkG\n"
"A1UECgwCQ0cxCzAJBgNVBAsMAklEMRIwEAYDVQQDDAlsb2NhbGhvc3QwHhcNMjMw\n"
"MjIwMTMwODEyWhcNMjgwMjIwMTMwODEyWjBVMQswCQYDVQQGEwJJTjELMAkGA1UE\n"
"CAwCS0ExCzAJBgNVBAcMAkJBMQswCQYDVQQKDAJDRzELMAkGA1UECwwCSUQxEjAQ\n"
"BgNVBAMMCWxvY2FsaG9zdDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB\n"
"ALxvFGAeqIcVT4x9rcPUIdQjQn/ied6DjMDSKrl5+GFKfVwUjHF5wSh1qh55WR/S\n"
"FVQEJ4yRzwNoRNp8YRYW6V6VUt7/gkh7Wwoct9WglFx5y0Wl7fQoQJu60gkEYS/B\n"
"CC3W0w2mcAUgm+aboUorNMz10o1TIkhKA/Qp4YKydmbkWSYFyXg+Ab+my4Fzo/Wp\n"
"/7/yfJJ1GDJgMBj3/BihDr5NDwJ9bW2ivCzgUCxifmsvj5dNuas6Rypgnc5LCrwR\n"
"JpvfLo3EP/F5Sr0zSa88c154QcJDlBZYy+AWUYRbH9kPfrAyP/Xf0zpqXNcYNEpu\n"
"k7gUSoIDDgm7gTqYcTj4IdECAwEAAaNTMFEwHQYDVR0OBBYEFKwhlzzFhGH4VUpS\n"
"t/oobYRE1U+xMB8GA1UdIwQYMBaAFKwhlzzFhGH4VUpSt/oobYRE1U+xMA8GA1Ud\n"
"EwEB/wQFMAMBAf8wDQYJKoZIhvcNAQELBQADggEBALii2W/LzxALqh6+rQWCgI+g\n"
"FRoysTeO3s9+95PTz5ctT7WFd7+lCJ/h4W8HXA1DIhaDi/OafV+8b+CVrvLZuw/5\n"
"PfUxCXNzqVfWWFUR8PrQc1mvGPOjqxuQdK0GuRywEezzFXbCfkS6Bg1YP0PD9Kt9\n"
"MHHo0I1+UJ4Ce3CnVjrc5/LNqqIxOTGen3Ww2i2tcTF7AWwLpAkEoSLFjzMzQtSz\n"
"aMjlC/3MiefJzoSa51wpNgqPjfMq7HibqbNyg+ectW/5S6FGgRAOlxSPm9hdzNWV\n"
"l47KOeR168TSKvFa4fslvKFgsmyFUPWGi9EpBshsXsCSiiWi0yaScHSj5yq/P5E=\n"
"-----END CERTIFICATE-----";

char device_certificate[] = "-----BEGIN CERTIFICATE-----\n"
"MIIBpzCCAU0CFAyhZKv2UJYX/5sFRapbXcMnWLZIMAoGCCqGSM49BAMCMFUxCzAJ\n"
"BgNVBAYTAklOMQswCQYDVQQIDAJLQTELMAkGA1UEBwwCQkExCzAJBgNVBAoMAkNH\n"
"MQswCQYDVQQLDAJDRTESMBAGA1UEAwwJbG9jYWxob3N0MB4XDTIzMDEyNTA2NDMw\n"
"NVoXDTI0MDEyMDA2NDMwNVowVzELMAkGA1UEBhMCSU4xCzAJBgNVBAgMAktBMQsw\n"
"CQYDVQQHDAJCQTELMAkGA1UECgwCQ0cxCzAJBgNVBAsMAklEMRQwEgYDVQQDDAtt\n"
"cXR0LWNsaWVudDBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABKOGGPfEgSpQotAE\n"
"u8lI8hkZEPS8qiqDAyaYZnyd6Qg2LsTzpU1KFwzUgIZQPuFJNnsoP8nSDlh/9/5E\n"
"NiqddLcwCgYIKoZIzj0EAwIDSAAwRQIhAIDQzIW+lRYPUzEHAcYDK8cuqdzFzgKN\n"
"Ye6O46Os34GDAiBt/ODIHL3yTYd3uyBGMk0NGolFnd6L3o/Sv7OZHgsESw==\n"
"-----END CERTIFICATE-----";

char device_key[] = "-----BEGIN EC PRIVATE KEY-----\n"
"MHcCAQEEIGdTdbc0E7+VQEjU17tPPB4jgX3sbz0oKPzoobcFtZimoAoGCCqGSM49\n"
"AwEHoUQDQgAEo4YY98SBKlCi0AS7yUjyGRkQ9LyqKoMDJphmfJ3pCDYuxPOlTUoX\n"
"DNSAhlA+4Uk2eyg/ydIOWH/3/kQ2Kp10tw==\n"
"-----END EC PRIVATE KEY-----";
 
#endif //for certificates

#endif // __DIC_CONFIG_H
