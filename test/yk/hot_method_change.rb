# Under yk the method lookup is an idempotent call keyed on `mrb_yk_method_version` (vm.c);
# this test checks that idempotent method lookup works when we define or remove a method dynamically.
#
# Run with: YKD_SERIALISE_COMPILATION=1 build/yk_build_config/bin/mruby <this file>
$outer_iters = 100
$inner_iters = 10000

klass = Class.new { def one; 1; end; def two; 2; end }
obj = klass.new

i = 0
while i < $outer_iters
  klass.alias_method(:m, :one)

  expected = 1
  j = 0
  while j < $inner_iters
    if j == $inner_iters / 2
      klass.alias_method(:m, :two)
      expected = 2
    end
    raise "iteration #{i}, #{j}" unless obj.m == expected
    j += 1
  end

  klass.remove_method(:m)
  begin
    obj.m
    raise "remove: iteration #{i}"
  rescue NoMethodError
  end

  i += 1
end

puts 'OK'
