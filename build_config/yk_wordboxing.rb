MRuby::Build.new('yk_wordboxing') do |conf|
  conf.toolchain :gcc

  plain_mrbc = "#{MRUBY_ROOT}/build/host/mrbc/bin/mrbc"
  unless File.exist?(plain_mrbc)
    fail "#{plain_mrbc} not found - run a plain `rake` first"
  end
  conf.mrbcfile = plain_mrbc

  yk_root = ENV['YK_ROOT'] || "#{MRUBY_ROOT}/../yk"
  yk_profile = ENV['YK_PROFILE'] || 'release'

  conf.cc.command = `#{yk_root}/bin/yk-config #{yk_profile} --cc`.strip
  conf.linker.command = conf.cc.command
  conf.cc.flags += `#{yk_root}/bin/yk-config #{yk_profile} --cflags --cppflags`.strip.split(' ')
  conf.linker.flags += `#{yk_root}/bin/yk-config #{yk_profile} --ldflags --libs`.strip.split(' ')
  conf.archiver.command = `#{yk_root}/bin/yk-config #{yk_profile} --ar`.strip
  conf.cc.defines << 'USE_YK'
  conf.cc.defines << 'MRB_NO_DIRECT_THREADING'
  # Without MRB_WORD_BOXING, mrb_value is a 2-word {union; enum tt} 
  # hitting yk's "Multi-locations not yet supported" error.
  conf.cc.defines << 'MRB_WORD_BOXING'
  conf.cc.defines << 'MRB_INT64'
  conf.cc.defines << 'MRB_CALL_LEVEL_MAX=4096'
  conf.gembox 'stdlib'
  conf.gembox 'stdlib-ext'
  conf.gembox 'stdlib-io'
  conf.gembox 'math'
  conf.gembox 'metaprog'
  conf.gem core: 'mruby-bin-mruby'

  conf.enable_test
end
