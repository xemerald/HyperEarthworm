
from urllib2 import quote
test_string="/maps/api/staticmap?key=AIzaSyDE6GfSr8sEPDAaQhovUXkKyJL0KeU8qRc&size=600x400&format=png8&maptype=hybrid&sensor=false&markers=icon:http:%2F%2Fmaps.google.com%2Fmapfiles%2Fkml%2Fpaddle%2Fylw-stars.png|shadow:true|40.302334,-80.846336&markers=icon:http:%2F%2Fmaps.google.com%2Fmapfiles%2Fkml%2Fshapes%2Fplacemark_circle.png|shadow:false|40.249200,-81.059100|40.083453,-80.900247|40.083453,-80.900247|40.228177,-81.145790|40.228177,-81.145790|40.201785,-81.200397|40.201785,-81.200397|40.041070,-81.009362|40.041070,-81.009362|40.247973,-81.267922|40.247973,-81.267922|40.182100,-80.377800|40.111210,-81.293472|39.886320,-80.445220|40.288737,-81.538200|40.131000,-81.509200|40.291298,-81.598200|39.791832,-81.523888|40.115800,-81.836100|39.486800,-81.389600|38.935595,-81.786445"



escaped_string = quote(test_string.encode('utf-8'))

print "original string:\n %s " % test_string
print "utf-8 string:\n %s " % escaped_string
