project('rtfilter', 'c',
        version : '1.2.0',
        default_options : [
            'warning_level=3',
        ],
        license : 'LGPL-3.0',
        meson_version: '>= 0.49'
)

# follow semantic versionning (https://semver.org)
# * MAJOR version when you make incompatible API changes,
# * MINOR version when you add functionality in a backwards-compatible manner
# * PATCH version when you make backwards-compatible bug fixes.
major = '1'
minor = '2'
patch = '0'
version = major + '.' + minor + '.' + patch

cc = meson.get_compiler('c')
configuration_inc = include_directories('.', 'src')

# additional (optional) warnings
flags = [
    '-Wshadow',
    '-Wstrict-prototypes',
    '-Wmissing-prototypes',
]
add_project_arguments(cc.get_supported_arguments(flags), language : 'c')

# define HAVE_CONFIG_H with compiler command line to include the generated
# config.h file (same as autotools)
add_project_arguments('-DHAVE_CONFIG_H', language : 'c')

config = configuration_data()
subdir('config/api-exports')

# Defined for compatibility with autotools
# The other such macros defined by autotools are not used and will not be added
config.set('PACKAGE_STRING', '"' + meson.project_name() + ' ' + meson.project_version() + '"')

# list of mandatory headers
check_headers = [
    'complex.h',
    'math.h',
]
foreach h : check_headers
    cc.check_header(h)
endforeach

# sse instructions handling
sse_common_sources = files('src/filter-funcs.h', 'src/filter-internal.h')
sse_libraries = []
if cc.has_header_symbol('cpuid.h', '__get_cpuid')
    config.set('HAVE_CPUID', 1)

    # sse support
    if cc.has_header_symbol('xmmintrin.h', '_mm_set1_ps')
        config.set('SUPPORT_SSE_SET', 1)
        sse_libraries += static_library('rtfilter sse filter implementation',
                sse_common_sources + files('src/filter-sse.c'),
                c_args : '-msse',
                include_directories : configuration_inc,
        )
    endif

    # sse2 support
    if cc.has_header_symbol('emmintrin.h', '_mm_set1_pd')
        config.set('SUPPORT_SSE2_SET', 1)
        sse_libraries += static_library('rtfilter sse2 filter implementation',
                sse_common_sources + files('src/filter-sse2.c'),
                c_args : '-msse2',
                include_directories : configuration_inc,
        )
    endif

    # sse3 support
    if cc.has_header_symbol('pmmintrin.h', '_mm_movehdup_ps')
        config.set('SUPPORT_SSE3_SET', 1)
        sse_libraries += static_library('rtfilter sse3 filter implementation',
                sse_common_sources + files('src/filter-sse3.c'),
                c_args : '-msse3',
                include_directories : configuration_inc,
        )
    endif
endif


# check posix_memalign presence
if cc.has_function('posix_memalign', prefix : '#include<stdlib.h>')
    config.set('HAVE_POSIX_MEMALIGN', 1)
else
    config.set('HAVE_POSIX_MEMALIGN', 0)
endif


# write config file
build_cfg = 'config.h'  # named as such to match autotools build system
configure_file(output : build_cfg, configuration : config)

headers = files(
        'src/rtfilter.h',
        'src/rtf_common.h',
)
sources = files(
        'src/common-filters.c',
        'src/downsampler.c',
        'src/filter.c',
        'src/filter-complex-double.c',
        'src/filter-complex-single.c',
        'src/filter-funcs.h',
        'src/filter-inreal-outcomplex-double.c',
        'src/filter-inreal-outcomplex-single.c',
        'src/filter-internal.h',
        'src/filter-real-double.c',
        'src/filter-real-single.c',
        'src/probesimd.h',
        'src/rtf_common.h',
        'src/rtfilter.h',
)

install_headers(headers)

libmath = cc.find_library('m', required : true)
rtfilter = shared_library('rtfilter',
        sources,
        install : true,
        version : version,
        include_directories : configuration_inc,
        link_with : sse_libraries,
        dependencies : [libmath],
)
import('pkgconfig').generate(rtfilter)

#
# TESTS
#

librt = []
perf_enabled = true
if get_option('perf').enabled()
    # clock_gettime() may be provided by librt or libc depending
    # on their versions.
    clock_gettime_code = ''' #include <time.h>
                             int clock_gettime();
                             int main(void){return 0;} '''
    if not cc.links(clock_gettime_code)
        if cc.links(clock_gettime_code, args : '-lrt')
            librt = cc.find_library('rt', required : get_option('perf'))
        elif get_option('perf').enabled()
            error('''clock_gettime() cannot be found, but is required to
                      build the perf-test tool !''')
        else
            perf_enabled = false
        endif
    endif
endif

if perf_enabled
    # test tool to ease performance regression tests
    # not run as part of unit tests
    perf_test = executable('perf-test',
            files('test/perf-test.c'),
            include_directories : configuration_inc,
            link_with : rtfilter,
            dependencies : [libmath, librt],
    )
endif

if get_option('tests').enabled()
    checklib = dependency('check', required : get_option('tests'))

    test_sources = files(
            'test/float-comparison.c',
            'test/float-comparison.h',
            'test/lowpass_test.c',
            'test/rtfilter_combine_test.c',
            'test/rtfilter_test.c',
            'test/testcases.h',
            'test/test-common.c',
            'test/test-common.h',
            'test/unittests.c'
    )
    unittests = executable('unittests',
            test_sources,
            include_directories : include_directories('.', 'src', 'test'),
            link_with : rtfilter,
            dependencies: [libmath, checklib],
    )
    test('rtfilter unit tests', unittests)
endif

#
# DOC
#

install_man(files('doc/rtf_create_bandpass_analytic.3',
        'doc/rtf_create_butterworth.3',
        'doc/rtf_create_chebychev.3',
        'doc/rtf_create_downsampler.3',
        'doc/rtf_create_filter.3',
        'doc/rtf_create_filter_coeffs.3',
        'doc/rtf_create_fir_bandpass.3',
        'doc/rtf_create_fir_highpass.3',
        'doc/rtf_create_fir_lowpass.3',
        'doc/rtf_destroy_filter.3',
        'doc/rtf_filter.3',
        'doc/rtf_filter_combine.3',
        'doc/rtf_filter_set_lazy_init.3',
        'doc/rtf_get_type.3',
        'doc/rtf_get_version.3',
        'doc/rtf_init_filter.3',
))

# compile example as a test, only install the sources
example = executable('documentation example',
        files('doc/examples/butterworth.c'),
        include_directories : include_directories('.', 'src'),
        link_with : rtfilter,
)

install_subdir('doc/examples',
        install_dir :  get_option('datadir') / 'doc' / 'rtfilter')

install_data(files('AUTHORS', 'COPYING', 'NEWS', 'README.md'),
        install_dir : get_option('datadir') / 'doc' / 'rtfilter')

#
# DEVTOOLS
#

uncrustify = find_program('uncrustify', required : false)
if uncrustify.found()
    run_target('checkstyle',
        command : [
            uncrustify,
            '-l', 'c',
            '-c', join_paths(meson.source_root(), 'devtools', 'uncrustify.cfg'),
            '--check',
            sources,
        ],
    )
    run_target('fixstyle',
        command : [
            uncrustify,
            '-l', 'c',
            '-c', join_paths(meson.source_root(), 'devtools', 'uncrustify.cfg'),
            '--replace',
            sources,
        ],
    )
endif # uncrustify

codespell = find_program('codespell', required : false)
if codespell.found()
    run_target('spelling',
        command : [
            codespell,
            sources,
        ]
    )
endif # codespell

# test public headers for most warnings and C++ compatibility
# only test rtfilter.h and not all exported headers: $(include_HEADERS)
# "rtf_common.h" contains a #warning which would always make the test fail
# otherwise
run_target('api-compat-test',
        command : [
            'bash',
            files('devtools/api-compat-test.sh'),
            meson.current_source_dir() / 'src',
            files('src/rtfilter.h'),
        ]
)
