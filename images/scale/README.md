# Defining VU meter scale

To convert from _VU_ scale to _dBu_ we just add 4, so 0VU becomes +4dBu. _dBu_ scale can converted to V<sub>RMS</sub> using equation:
![U_Vrms = sqrt(0.6) * 10^(U_dBu/20)](http://www.sciweavers.org/tex2img.php?eq=U_%7BV_%7Brms%7D%7D%20%3D%20%5Csqrt%7B0.6%7D%20%5Ctimes%2010%5E%7B%5Cfrac%7BU_%7BdBu%7D%7D%7B20%7D%7D&bc=White&fc=Black&im=jpg&fs=12&ff=arev&edit=0)

If we assume that VU meter scale will range from 0V<sub>rms</sub> to 1.734V<sub>rms</sub> (+3VU) and the needle can from vertical position by +-42° we get following angles for VU values:

| U [VU] | U [dBu] | U [V<sub>rms</sub>] | angle [°] |
|--:|--:|--:|--:|
|-20|-16|0.123V|-35°|
|-19|-15|0.138|-34|
|-18|-14|0.155|-33|
|-17|-13|0.173|-32|
|-16|-12|0.195|-32|
|-15|-11|0.218|-30|
|-14|-10|0.245|-29|
|-13| -9|0.275|-28|
|-12| -8|0.308|-26|
|-11| -7|0.346|-24|
|-10| -6|0.388|-22|
| -9| -5|0.436|-20|
| -8| -4|0.489|-18|
| -7| -3|0.548|-15|
| -6| -2|0.615|-12|
| -5| -1|0.690| -8|
| -4| +0|0.775| -4|
| -3| +1|0.869|  0|
| -2| +2|0.975|  5|
| -1| +3|1.094| 11|
| +0| +4|1.228| 17|
| +1| +5|1.377| 24|
| +2| +6|1.546| 32|
| +3| +7|1.734| 41|

For percentage scale we get the following:

| U [% 0dBu] | U [V<sub>rms</sub>] | angle [°] |
|--:|--:|--:|
|0|0.000|-41|
|5|0.061|-38|
|10|0.123|-35|
|15|0.184|-32|
|20|0.246|-29|
|25|0.307|-26|
|30|0.368|-23|
|35|0.430|-20|
|40|0.491|-18|
|45|0.552|-15|
|50|0.614|-12|
|55|0.675| -9|
|60|0.737| -6|
|65|0.798| -3|
|70|0.859|  0|
|75|0.921|  3|
|80|0.982|  5|
|85|1.044|  8|
|90|1.105| 11|
|95|1.166| 14|
|100|1.228| 17|

