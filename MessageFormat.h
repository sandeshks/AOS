#ifndef MESSAGEFORMAT_H
#define MESSAGEFORMAT_H

#pragma once
class MessageFormat
{
public:
	MessageFormat(void);
	~MessageFormat(void);

	struct AlgoMsg{
		int TYPE;
		int ORIGIN;
		long SEQ;
		int sender;
	}
};

#endif
