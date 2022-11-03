total=dict()
l=[418, 690, 1536, 160, 1552, 1312, 1538, 320, 1537, 1281, 1282, 1602, 1138, 274, 978, 561, 99, 1008, 480, 144, 1264, 83, 130, 385, 1424, 674]

for item in l:
    endswith=eval('0b'+(f'{item:b}'[-2:]))
    if endswith in total:
        total[endswith].append(item);
    else:
        total[endswith] = [item]

    
x = {k:v for k,v in sorted(list(total.items()))}
print([j for i in x.values() for j in i])