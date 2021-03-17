POST /example.com/nsmf-pdusession/v1/sm-contexts HTTP/2

Content-Type: multipart/related; type="application/json"; boundary=thisIsABoundary

--thisIsABoundary

Content-Type: application/json

{
"supi": "imsi-<IMSI>",
"pduSessionId": 235,
"dnn": "<DNN>",
"sNssai": {
"sst": 0
},
"servingNfId": "<AMF Identifier>",
"n1SmMsg": {
"contentId": "n1msg"
},
"anType": "3GPP_ACCESS",
"smContextStatusUri": "<URI>"
}

--thisIsABoundary

Content-Type: application/vnd.3gpp.5gnas
Content-Id: n1msg
{ … N1 SM Message binary data …}
--thisIsABoundary--