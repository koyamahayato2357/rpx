const std = @import("std");
const allocor = std.heap.page_allocator;
const srcdir: []const u8 = "src";
const incdir: []const u8 = "include";
const cflags_default: []const []const u8 = &[_][]const u8{
    "-std=c2y",
    "-Wall",
    "-Wextra",
    "-Werror",
    "-Wimplicit-fallthrough",
    "-Wbitwise-instead-of-logical",
    "-Wconversion",
    "-Wdangling",
    "-Wdeprecated",
    "-Wdocumentation",
    "-Wmicrosoft",
    "-Wswitch-enum",
    "-Wswitch-default",
    "-Wtype-limits",
    "-Wunreachable-code-aggressive",
    "-Wpedantic",
    "-Wdocumentation-pedantic",
    "-Wno-dollar-in-identifier-extension",
    "-Wno-gnu",
};

pub fn build(b: *std.Build) void {
    var cflags = std.ArrayList([]const u8).init(std.heap.page_allocator);
    defer cflags.deinit();
    cflags.appendSlice(cflags_default) catch unreachable;

    const targ = b.standardTargetOptions(.{});
    const opti = b.standardOptimizeOption(.{});
    const exe = b.addExecutable(.{
        .name = "rpx",
        .optimize = opti,
        .target = targ,
        .use_lld = true,
        .use_llvm = true,
    });

    exe.addIncludePath(b.path(incdir));
    exe.linkLibC();

    // build type settings
    const build_type: []const u8 = b.option([]const u8, "T", "Build type") orelse "";
    if (std.mem.eql(u8, build_type, "test")) {
        exe.root_module.addCMacro("TEST_MODE", "");
    } else if (std.mem.eql(u8, build_type, "bench")) {
        exe.root_module.addCMacro("BENCHMARK_MODE", "");
    }

    addSourceFromDir(exe, b.path(srcdir), cflags.items);

    b.installArtifact(exe);

    const run_exe = b.addRunArtifact(exe);
    const run_step = b.step("run", "Run the code");
    run_step.dependOn(&run_exe.step);
}

fn addSourceFromDir(exe: *std.Build.Step.Compile, dir: std.Build.LazyPath, cflags: []const []const u8) void {
    var diren = std.fs.cwd().openDir(dir.src_path.sub_path, .{ .iterate = true }) catch unreachable;
    defer diren.close();
    var files = std.ArrayList([]const u8).init(std.heap.page_allocator);
    var srcs = diren.iterate();
    while (srcs.next() catch unreachable) |src|
        if (std.mem.eql(u8, std.fs.path.extension(src.name), ".c"))
            files.append(src.name) catch unreachable;

    exe.addCSourceFiles(.{
        .files = files.items, // memory leak 🤣🤣🤣
        .flags = cflags,
        .language = .c,
        .root = dir,
    });
}
