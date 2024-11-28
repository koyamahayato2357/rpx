const std = @import("std");
const allocor = std.heap.page_allocator;
const srcdir: []const u8 = "src";
const incdir: []const u8 = "include";
const cflags: []const []const u8 = &[_][]const u8{
    "-std=c23",
};

pub fn build(b: *std.Build) void {
    const exe = b.addExecutable(.{ .name = std.fs.path.basename(b.build_root.path.?), .target = b.host, .use_lld = true });
    exe.addIncludePath(b.path(incdir));
    exe.linkLibC();

    var srcs = std.fs.cwd().openDir(srcdir, .{ .iterate = true }) catch unreachable;
    defer srcs.close();

    var srciter = srcs.iterate();
    while (srciter.next() catch unreachable) |src|
        if (std.mem.eql(u8, std.fs.path.extension(src.name), ".c")) {
            const paths: []const []const u8 = &[_][]const u8{ srcdir, src.name };
            // .file = srcdir ++ src.name
            exe.addCSourceFile(.{ .file = .{ .cwd_relative = std.fs.path.join(allocor, paths) catch unreachable }, .flags = cflags });
        };

    b.installArtifact(exe);

    const run_exe = b.addRunArtifact(exe);
    const run_step = b.step("run", "Run the code");
    run_step.dependOn(&run_exe.step);
}
