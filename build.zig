const std = @import("std");
const allocor = std.heap.page_allocator;
const srcdir: []const u8 = "src";
const incdir: []const u8 = "include";
const cflags: []const []const u8 = &[_][]const u8{
    "-std=c23",
    "-Wall",
    "-Wextra",
    "-Werror",
};

pub fn build(b: *std.Build) void {
    const exe = b.addExecutable(.{ .name = std.fs.path.basename(b.build_root.path.?), .target = b.host, .use_lld = true });
    exe.addIncludePath(b.path(incdir));
    exe.linkLibC();

    const build_type: []const u8 = b.option([]const u8, "T", "Build type") orelse "";
    if (std.mem.eql(u8, build_type, "test")) {
        exe.defineCMacro("TEST_MODE", null);
    } else if (std.mem.eql(u8, build_type, "bench")) {
        exe.defineCMacro("BENCHMARK_MODE", null);
    }

    addSourceFromDir(exe, srcdir);

    b.installArtifact(exe);

    const run_exe = b.addRunArtifact(exe);
    const run_step = b.step("run", "Run the code");
    run_step.dependOn(&run_exe.step);
}

fn addSourceFromDir(exe: *std.Build.Step.Compile, dir: []const u8) void {
    var diren = std.fs.cwd().openDir(dir, .{ .iterate = true  }) catch unreachable;
    defer diren.close();
    var srcs = diren.iterate();
    while (srcs.next() catch unreachable) |src| {
        if (!std.mem.eql(u8, std.fs.path.extension(src.name), ".c"))
            break;

        const path: []const []const u8 = &[_][]const u8{ srcdir, src.name };
        // .file = srcdir ++ src.name
        exe.addCSourceFile(.{ .file = .{ .cwd_relative = std.fs.path.join(allocor, path) catch unreachable }, .flags = cflags });
    }
}
