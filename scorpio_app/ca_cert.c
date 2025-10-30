/**
 * @file ca_cert.c
 * @author Vetrivel
 * @brief 
 * @date 2025-05-15
 * @copyright Copyright (c) 2025
 */
/************************************************************* Header includes ***************************************************************/
#include "ca_cert.h"
/***************************************************************** Macros ********************************************************************/

/*************************************************************** Variables ******************************************************************/
const uint8_t au8_root_certificate[] =
#if (AWS_SERVER)
"-----BEGIN CERTIFICATE-----\n"
"MIIEXjCCA0agAwIBAgITB3MSTNQG0mfAmRzdKZqfODF5hTANBgkqhkiG9w0BAQsF\n"
"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n"
"b24gUm9vdCBDQSAxMB4XDTIyMDgyMzIyMjYwNFoXDTMwMDgyMzIyMjYwNFowPDEL\n"
"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEcMBoGA1UEAxMTQW1hem9uIFJT\n"
"QSAyMDQ4IE0wMzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALd/pVko\n"
"8vuM475Tf45HV3BbCl/B9Jy89G1CRkFjcPY06WA9lS+7dWbUA7GtWUKoksr69hKM\n"
"wcMsNpxlw7b3jeXFgxB09/nmalcAWtnLzF+LaDKEA5DQmvKzuh1nfIfqEiKCQSmX\n"
"Xh09Xs+dO7cm5qbaL2hhNJCSAejciwcvOFgFNgEMR42wm6KIFHsQW28jhA+1u/M0\n"
"p6fVwReuEgZfLfdx82Px0LJck3lST3EB/JfbdsdOzzzg5YkY1dfuqf8y5fUeZ7Cz\n"
"WXbTjujwX/TovmeWKA36VLCz75azW6tDNuDn66FOpADZZ9omVaF6BqNJiLMVl6P3\n"
"/c0OiUMC6Z5OfKcCAwEAAaOCAVowggFWMBIGA1UdEwEB/wQIMAYBAf8CAQAwDgYD\n"
"VR0PAQH/BAQDAgGGMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjAdBgNV\n"
"HQ4EFgQUVdkYX9IczAHhWLS+q9lVQgHXLgIwHwYDVR0jBBgwFoAUhBjMhTTsvAyU\n"
"lC4IWZzHshBOCggwewYIKwYBBQUHAQEEbzBtMC8GCCsGAQUFBzABhiNodHRwOi8v\n"
"b2NzcC5yb290Y2ExLmFtYXpvbnRydXN0LmNvbTA6BggrBgEFBQcwAoYuaHR0cDov\n"
"L2NydC5yb290Y2ExLmFtYXpvbnRydXN0LmNvbS9yb290Y2ExLmNlcjA/BgNVHR8E\n"
"ODA2MDSgMqAwhi5odHRwOi8vY3JsLnJvb3RjYTEuYW1hem9udHJ1c3QuY29tL3Jv\n"
"b3RjYTEuY3JsMBMGA1UdIAQMMAowCAYGZ4EMAQIBMA0GCSqGSIb3DQEBCwUAA4IB\n"
"AQAGjeWm2cC+3z2MzSCnte46/7JZvj3iQZDY7EvODNdZF41n71Lrk9kbfNwerK0d\n"
"VNzW36Wefr7j7ZSwBVg50W5ay65jNSN74TTQV1yt4WnSbVvN6KlMs1hiyOZdoHKs\n"
"KDV2UGNxbdoBYCQNa2GYF8FQIWLugNp35aSOpMy6cFlymFQomIrnOQHwK1nvVY4q\n"
"xDSJMU/gNJz17D8ArPN3ngnyZ2TwepJ0uBINz3G5te2rdFUF4i4Y3Bb7FUlHDYm4\n"
"u8aIRGpk2ZpfXmxaoxnbIBZRvGLPSUuPwnwoUOMsJ8jirI5vs2dvchPb7MtI1rle\n"
"i02f2ivH2vxkjDLltSpe2fiC\n"
"-----END CERTIFICATE-----\0";

#else
  "-----BEGIN CERTIFICATE-----\n"          
  "MIIDdTCCAl2gAwIBAgILBAAAAAABFUtaw5QwDQYJKoZIhvcNAQEFBQAwVzELMAkG\n"
  "A1UEBhMCQkUxGTAXBgNVBAoTEEdsb2JhbFNpZ24gbnYtc2ExEDAOBgNVBAsTB1Jv\n"
  "b3QgQ0ExGzAZBgNVBAMTEkdsb2JhbFNpZ24gUm9vdCBDQTAeFw05ODA5MDExMjAw\n"
  "MDBaFw0yODAxMjgxMjAwMDBaMFcxCzAJBgNVBAYTAkJFMRkwFwYDVQQKExBHbG9i\n"
  "YWxTaWduIG52LXNhMRAwDgYDVQQLEwdSb290IENBMRswGQYDVQQDExJHbG9iYWxT\n"
  "aWduIFJvb3QgQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDaDuaZ\n"
  "jc6j40+Kfvvxi4Mla+pIH/EqsLmVEQS98GPR4mdmzxzdzxtIK+6NiY6arymAZavp\n"
  "xy0Sy6scTHAHoT0KMM0VjU/43dSMUBUc71DuxC73/OlS8pF94G3VNTCOXkNz8kHp\n"
  "1Wrjsok6Vjk4bwY8iGlbKk3Fp1S4bInMm/k8yuX9ifUSPJJ4ltbcdG6TRGHRjcdG\n"
  "snUOhugZitVtbNV4FpWi6cgKOOvyJBNPc1STE4U6G7weNLWLBYy5d4ux2x8gkasJ\n"
  "U26Qzns3dLlwR5EiUWMWea6xrkEmCMgZK9FGqkjWZCrXgzT/LCrBbBlDSgeF59N8\n"
  "9iFo7+ryUp9/k5DPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNVHRMBAf8E\n"
  "BTADAQH/MB0GA1UdDgQWBBRge2YaRQ2XyolQL30EzTSo//z9SzANBgkqhkiG9w0B\n"
  "AQUFAAOCAQEA1nPnfE920I2/7LqivjTFKDK1fPxsnCwrvQmeU79rXqoRSLblCKOz\n"
  "yj1hTdNGCbM+w6DjY1Ub8rrvrTnhQ7k4o+YviiY776BQVvnGCv04zcQLcFGUl5gE\n"
  "38NflNUVyRRBnMRddWQVDf9VMOyGj/8N7yy5Y0b2qvzfvGn9LhJIZJrglfCm7ymP\n"
  "AbEVtQwdpf5pLGkkeB6zpxxxYu7KyJesF12KwvhHhm4qxFYxldBniYUr+WymXUad\n"
  "DKqC5JlR3XC321Y9YeRq4VzW9v493kHMB65jUr9TU/Qr6cf9tveCX4XSQRjbgbME\n"
  "HMUfpIBvFSDJ3gyICh3WZlXi/EjJKSZp4A==\n"
  "-----END CERTIFICATE-----\n";
#endif
