#pragma once

namespace implementation
{
	struct event;
	struct mapped_file;
	struct memory_block;
	struct memory_cache;
	struct message;
	struct message_event;
	struct message_queue;
	struct receive;
	struct server;
	struct server_cache;
	struct trigger;
	struct trigger_cache;
	struct trigger_event;
};

namespace xml
{
	struct pull;
	struct receive;
	struct message;
};

namespace raw
{
	struct pull;
	struct receive;
	struct message;
};

namespace audio
{
	struct pull;
	struct receive;
	struct message;
};

namespace video
{
	struct pull;
	struct receive;
	struct message;
};

namespace debug
{
	struct pull;
	struct receive;
	struct message;
};

namespace audio_video
{
	struct receive;
};

namespace audio_video_xml
{
	struct receive;
};