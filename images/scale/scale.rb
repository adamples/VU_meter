

def log10(x)
  return Math.log(x) / Math.log(10)
end


def _dbu_to_v(dbu)
  return 10**((dbu + 20 * log10(Math.sqrt(0.6))) / 20)
end


def dbu_to_v(dbu)
  #return 10**(dbu.to_f/20) * 10**((20 * log10(Math.sqrt(0.6))) / 20)
  return 10**(dbu.to_f/20) * Math.sqrt(0.6)
end


def vu_to_v(vu)
  return dbu_to_v(vu + 4)
end

def deg(rad)
  return 180.0 * rad / Math::PI
end


WIDTH = 128.0

V_MIN = 0.0
V_REF = vu_to_v(0.0)
V_MAX = vu_to_v(+3.0)

L = 96
MAX_ANGLE = Math.asin(((WIDTH.to_f - 1)/2 - 1).to_f / L)


(-20..6).each do |vu|
  v = vu_to_v(vu)
  percent = 100 * v / V_REF
  angle = (v / V_MAX - 0.5) * 2 * MAX_ANGLE
  #puts " %+3dVU:  v=%5.3fV  p=%5.1f%%  angle=%3ddeg" % [vu, v, percent, deg(angle).round()]
  puts "|%+3d|%+3d|%5.3f|%3d|" % [vu, vu + 4, v, deg(angle).round()]
end

puts

(0..100).step(5) do |percent|
  v = percent.to_f / 100 * V_REF
  angle = (v / V_MAX - 0.5) * 2 * MAX_ANGLE
  puts "|%d|%5.3f|%3d|" % [percent, v, deg(angle).round()]
end
