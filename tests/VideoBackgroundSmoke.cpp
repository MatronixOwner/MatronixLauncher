// SPDX-License-Identifier: GPL-3.0-only
// Smoke test the packaged Qt multimedia backend and CPU frame conversion.
#include <QApplication>
#include <QImage>
#include <QMediaPlayer>
#include <QPainter>
#include <QTimer>
#include <QUrl>
#include <QVideoFrame>
#include <QVideoSink>
#include <cstdio>

int main(int argc, char** argv)
{
    qputenv("QT_FFMPEG_DECODING_HW_DEVICE_TYPES", ",");
    QApplication app(argc, argv);
    if (argc != 2)
        return 2;
    QMediaPlayer player;
    QVideoSink sink;
    player.setVideoSink(&sink);
    player.setLoops(QMediaPlayer::Infinite);
    QObject::connect(&sink, &QVideoSink::videoFrameChanged, &app, [&](const QVideoFrame& frame) {
        const QImage image = frame.toImage();
        if (image.isNull())
            return;
        QImage canvas(200, 120, QImage::Format_ARGB32);
        canvas.fill(Qt::transparent);
        QPainter painter(&canvas);
        painter.drawImage(canvas.rect(), image);
        painter.end();
        if (canvas.pixelColor(100, 60).alpha() == 0)
            return;
        std::puts("Received and painted a video frame");
        app.exit(0);
    });
    QObject::connect(&player, &QMediaPlayer::errorOccurred, &app, [&](QMediaPlayer::Error, const QString& message) {
        std::fprintf(stderr, "Playback failed: %s\n", qPrintable(message));
        app.exit(1);
    });
    QTimer::singleShot(30000, &app, [&] {
        std::fputs("No usable video frame received\n", stderr);
        app.exit(1);
    });
    const QUrl source = QUrl::fromLocalFile(QString::fromLocal8Bit(argv[1]));
    QTimer::singleShot(0, &app, [&] {
        player.setSource(source);
        player.play();
    });
    return app.exec();
}
