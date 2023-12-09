function sign(key, msg) {
    return CryptoJS.HmacSHA256(msg, key);
}

function getSignatureKey(key, dateStamp, regionName, serviceName, terminator) {
    let kDate = sign("AWS4" + key, dateStamp);
    let kRegion = sign(kDate, regionName);
    let kService = sign(kRegion, serviceName);
    let kSigning = sign(kService, terminator);
    return kSigning;
}

function get_header() {
    const now = new Date().toISOString();
    const amz_date = now.split('.')[0].replace(/[:-]/g, '') + 'Z';    // change format to YYYYMMDD'T'HHMMSS'Z'
    const date_stamp = now.split('T')[0].replace(/-/g, '');        // change format to YYYYMMDD

    const method = 'GET';
    const content_type = 'application/json';

    const host = 'a1fmltb7n8klk1-ats.iot.ap-northeast-2.amazonaws.com';
    const canonical_uri = '/things/ESP32_BME280_LED/shadow';
    const canonical_querystring = '';
    const request_parameters = '';

    const region_name = 'ap-northeast-2';
    const service_name = 'iotdata';
    const access_key = 'AKIA4TP7DHY4FSGI5TMF';
    const secret_key = '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';
    const terminator = 'aws4_request';

    const payload_hash = CryptoJS.SHA256(request_parameters).toString();

    const canonical_headers = `content-type:${content_type}\nhost:${host}\nx-amz-date:${amz_date}\n`;
    const signed_headers = `content-type;host;x-amz-date`;

    const canonical_request = `${method}\n${canonical_uri}\n${canonical_querystring}\n${canonical_headers}\n${signed_headers}\n${payload_hash}`;

    const algorithm = `AWS4-HMAC-SHA256`;
    const credential_scope = `${date_stamp}/${region_name}/${service_name}/${terminator}`;
    const string_to_sign = `${algorithm}\n${amz_date}\n${credential_scope}\n${CryptoJS.SHA256(canonical_request)}`;

    const signing_key = getSignatureKey(secret_key, date_stamp, region_name, service_name, terminator);
    const signature = sign(signing_key, string_to_sign).toString(CryptoJS.enc.Hex);

    const authorization_header = `${algorithm} Credential=${access_key}/${credential_scope}, SignedHeaders=${signed_headers}, Signature=${signature}`;

    const headers = {
        'Content-Type': content_type,
        'X-Amz-Date': amz_date,
        'Authorization': authorization_header
    }

    return headers;
}
function test() {
    fetch('https://a1fmltb7n8klk1-ats.iot.ap-northeast-2.amazonaws.com/things/ESP32_BME280_LED/shadow', {
        method: 'GET',
        headers: get_header()
    })
        .then(res => res.json())
        .then(data => console.log(data.state))
        .catch(err => console.log(err));
}

function post_header() {
    const now = new Date().toISOString();
    const amz_date = now.split('.')[0].replace(/[:-]/g, '') + 'Z';    // change format to YYYYMMDD'T'HHMMSS'Z'
    const date_stamp = now.split('T')[0].replace(/-/g, '');        // change format to YYYYMMDD

    const method = 'POST';
    const content_type = 'application/json';

    const host = 'a1fmltb7n8klk1-ats.iot.ap-northeast-2.amazonaws.com';
    const canonical_uri = '/things/ESP32_BME280_LED/shadow';
    const canonical_querystring = '';
    const request_parameters = JSON.stringify({
        "state": {
            "reported": {
                "temp": 30
            }
        }
    });

    const region_name = 'ap-northeast-2';
    const service_name = 'iotdata';
    const access_key = 'AKIA4TP7DHY4FSGI5TMF';
    const secret_key = '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';
    const terminator = 'aws4_request';

    const payload_hash = CryptoJS.SHA256(request_parameters).toString();

    const canonical_headers = `content-type:${content_type}\nhost:${host}\nx-amz-date:${amz_date}\n`;
    const signed_headers = `content-type;host;x-amz-date`;

    const canonical_request = `${method}\n${canonical_uri}\n${canonical_querystring}\n${canonical_headers}\n${signed_headers}\n${payload_hash}`;

    const algorithm = `AWS4-HMAC-SHA256`;
    const credential_scope = `${date_stamp}/${region_name}/${service_name}/${terminator}`;
    const string_to_sign = `${algorithm}\n${amz_date}\n${credential_scope}\n${CryptoJS.SHA256(canonical_request)}`;

    const signing_key = getSignatureKey(secret_key, date_stamp, region_name, service_name, terminator);
    const signature = sign(signing_key, string_to_sign).toString(CryptoJS.enc.Hex);

    const authorization_header = `${algorithm} Credential=${access_key}/${credential_scope}, SignedHeaders=${signed_headers}, Signature=${signature}`;

    const headers = {
        'Content-Type': content_type,
        'X-Amz-Date': amz_date,
        'Authorization': authorization_header
    }

    return headers;
}
function test2() {
    fetch('https://a1fmltb7n8klk1-ats.iot.ap-northeast-2.amazonaws.com/things/ESP32_BME280_LED/shadow', {
        method: 'POST',
        headers: post_header(),
        body: JSON.stringify({
            "state": {
                "reported": {
                    "temp": 30
                }
            }
        })
    })
        .then(res => res.json())
        .then(data => console.log(data))
        .catch(err => console.log(err));
}

