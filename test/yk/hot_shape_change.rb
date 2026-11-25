# This test checks that under yk the idempotent shape lookup returns the right value
# when the receiver's shape changes.
#
# Run with: YKD_SERIALISE_COMPILATION=1 build/yk_build_config/bin/mruby <this file>
$iters = 2000

class Pair
  def a; @a; end
  def a=(v); @a = v; end
  def b; @b; end
  def b=(v); @b = v; end
end

x = Pair.new; x.a = 0; x.b = -1
y = Pair.new; y.b = -2; y.a = 0

obj = x
expected_b = -1
i = 0
while i < $iters
  if i == $iters / 2
    obj = y
    expected_b = -2
  end
  obj.a = i
  raise "iteration #{i}" unless obj.a == i && obj.b == expected_b
  i += 1
end

puts 'OK'
