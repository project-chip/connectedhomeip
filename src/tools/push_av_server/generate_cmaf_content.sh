# source https://github.com/nagare-media/ingest/blob/main/scripts/tasks/run-cmaf-long-upload-ffmpeg
# Copyright 2022-2024 The nagare media authors under Apache 2.0

PUBLISHING_ENDPOINT=${PUBLISHING_ENDPOINT:-https://localhost:1234/stream/1}

# TODO Handle dynamic value for the certificates
CERT_ROOT_DIR=~/.pavstest

ffmpeg -hide_banner \
    -re -f lavfi -i "
    testsrc2=size=1280x720:rate=25,
    drawbox=x=0:y=0:w=700:h=50:c=black@.6:t=fill,
    drawtext=x=  5:y=5:fontsize=54:fontcolor=white:text='%{pts\:gmtime\:$(date +%s)\:%Y-%m-%d}',
    drawtext=x=345:y=5:fontsize=54:fontcolor=white:timecode='$(date -u '+%H\:%M\:%S')\:00':rate=25:tc24hmax=1,
    setparams=field_mode=prog:range=tv:color_primaries=bt709:color_trc=bt709:colorspace=bt709,
    format=yuv420p" \
    -re -f lavfi -i "
    sine=f=1000:r=48000:samples_per_frame='st(0,mod(n,5)); 1602-not(not(eq(ld(0),1)+eq(ld(0),3)))'" \
    -shortest \
    -fflags genpts \
    \
    -filter_complex "
    [0:v]drawtext=x=(w-text_w)-5:y=5:fontsize=54:fontcolor=white:text='720p':box=1:boxcolor=black@.6:boxborderw=5
  " \
    \
    -c:a aac \
    -b:a 64k \
    -c:v libx264 \
    -preset:v veryfast \
    -tune zerolatency \
    -profile:v main \
    -crf:v 23 -bufsize:v:0 2250k -maxrate:v 2500k \
    -g:v 100000 -keyint_min:v 50000 -force_key_frames:v "expr:gte(t,n_forced*2)" \
    -x264opts no-open-gop=1 \
    -bf 2 -b_strategy 2 -refs 1 \
    -rc-lookahead 24 \
    -export_side_data prft \
    -field_order progressive -colorspace bt709 -color_primaries bt709 -color_trc bt709 -color_range tv \
    -pix_fmt yuv420p \
    -f mp4 \
    -frag_duration "$((1 * 1000 * 1000))" \
    -min_frag_duration "$((1 * 1000 * 1000))" \
    -write_prft wallclock \
    -use_editlist 0 \
    -movflags "+cmaf+dash+delay_moov+skip_sidx+skip_trailer+frag_custom" \
    \
    -method PUT \
    -multiple_requests 1 \
    -chunked_post 1 \
    -send_expect_100 1 \
    -headers "DASH-IF-Ingest: 1.1" \
    -headers "Host: localhost:8080" \
    -content_type "" \
    -icy 0 \
    -rw_timeout "$((200 * 1000 * 1000))" \
    -reconnect 1 \
    -reconnect_at_eof 1 \
    -reconnect_on_network_error 1 \
    -reconnect_on_http_error 4xx,5xx \
    -reconnect_delay_max 2 \
    -ca_file "$CERT_ROOT_DIR/certs/server/root.pem" \
    -cert_file "$CERT_ROOT_DIR/certs/device/dev.pem" \
    -key_file "$CERT_ROOT_DIR/certs/device/dev.key" \
    -tls_verify 1 \
    "$PUBLISHING_ENDPOINT/segment1/video/clip1.m4s"
