/*
 * Copyright (C) 2016 The Jappsy Open Source Project (http://jappsy.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "uURI.h"
#include <wchar.h>
#include <data/uString.h>

const wchar_t* URI::basePath = L"http://localhost";

URI::URI(const wchar_t* uri) {
	_scheme = NULL;
	_server = NULL;
	_fullPath = NULL;
	_absolutePath = NULL;
	_uri = NULL;
	
	m_scheme = NULL;
	m_user = NULL;
	m_password = NULL;
	m_host = NULL;
	m_port = NULL;
	m_path = NULL;
	m_file = NULL;
	m_ext = NULL;
	m_query = NULL;
	m_anchor = NULL;
	
	if (uri != NULL) {
		wchar_t* s = (wchar_t*)uri;
		wchar_t* p = s;
		wchar_t ch;
		
		// Scheme
		do {
			ch = *p;
			if ((ch == L':') || (ch == L'/') || (ch == '?') || (ch == '#') || (ch == 0)) {
				if (ch == L':') {
					int len = ((intptr_t)p - (intptr_t)s) / sizeof(wchar_t);
					if (len > 0) {
						m_scheme = memAlloc(wchar_t, m_scheme, (len + 1) * sizeof(wchar_t));
						memcpy(m_scheme, s, len * sizeof(wchar_t));
						m_scheme[len] = 0;
					}
					s = p;
				}
				break;
			}
			p++;
		} while (true);
		
		// Server
		if ((s[0] == L':') && (s[1] == L'/') && (s[2] == L'/')) {
			s += 3;
			
			// User : Password
			p = s;
			do {
				ch = *p;
				if ((ch == L':') || (ch == L'@') || (ch == 0)) {
					wchar_t* u = p;
					if (ch == L':') {
						p++;
						
						do {
							ch = *p;
							if ((ch == L':') || (ch == L'@') || (ch == 0)) {
								break;
							}
							p++;
						} while (true);
					}
					
					if (ch == L'@') {
						int len = ((intptr_t)u - (intptr_t)s) / sizeof(wchar_t);
						if (len > 0) {
							m_user = memAlloc(wchar_t, m_user, (len + 1) * sizeof(wchar_t));
							memcpy(m_user, s, len * sizeof(wchar_t));
							m_user[len] = 0;
						}
						
						if (u != p) {
							int len = ((intptr_t)p - (intptr_t)u) / sizeof(wchar_t) - 1;
							
							if (len > 0) {
								m_password = memAlloc(wchar_t, m_password, (len + 1) * sizeof(wchar_t));
								memcpy(m_password, u + 1, len * sizeof(wchar_t));
								m_password[len] = 0;
							}
						}
						
						s = p + 1;
					}
					break;
				}
				p++;
			} while (true);
			
			// Host : Port
			p = s;
			do {
				ch = *p;
				if ((ch == L':') || (ch == L'/') || (ch == L'?') || (ch == L'#') || (ch == 0)) {
					wchar_t* h = p;
					if (ch == L':') {
						p++;
						
						do {
							ch = *p;
							if ((ch < L'0') || (ch > L'9')) {
								break;
							}
							p++;
						} while (true);
					}
					
					if (h != s) {
						int len = ((intptr_t)h - (intptr_t)s) / sizeof(wchar_t);
						if (len > 0) {
							m_host = memAlloc(wchar_t, m_host, (len + 1) * sizeof(wchar_t));
							memcpy(m_host, s, len * sizeof(wchar_t));
							m_host[len] = 0;
						}
					}
					
					if (p != h) {
						int len = ((intptr_t)p - (intptr_t)h) / sizeof(wchar_t) - 1;
						
						if (len > 0) {
							m_port = memAlloc(wchar_t, m_port, (len + 1) * sizeof(wchar_t));
							memcpy(m_port, h + 1, len * sizeof(wchar_t));
							m_port[len] = 0;
						}
					}
					
					s = p;
					break;
				}
				p++;
			} while (true);
			
			s = p;
		}
		
		// Path
		wchar_t* pe = s;
		p = s;
		do {
			ch = *p;
			if ((ch == L'?') || (ch == L'#') || (ch == L'/') || (ch == 0)) {
				if (ch == L'/') {
					p++;
					pe = p;
					continue;
				}
				
				if (pe != s) {
					int len = ((intptr_t)pe - (intptr_t)s) / sizeof(wchar_t);
					
					if (len > 0) {
						m_path = memAlloc(wchar_t, m_path, (len + 1) * sizeof(wchar_t));
						memcpy(m_path, s, len * sizeof(wchar_t));
						m_path[len] = 0;
					}
					
					s = pe;
				}
				break;
			}
			p++;
		} while (true);

		// File
		p = s;
		wchar_t* e = NULL;
		do {
			ch = *p;
			if (ch == L'.')
				e = p + 1;
			if ((ch == L'?') || (ch == L'#') || (ch == 0)) {
				int len = ((intptr_t)p - (intptr_t)s) / sizeof(wchar_t);
				
				if (len > 0) {
					m_file = memAlloc(wchar_t, m_file, (len + 1) * sizeof(wchar_t));
					memcpy(m_file, s, len * sizeof(wchar_t));
					m_file[len] = 0;
					
					if (e != NULL) {
						int len = ((intptr_t)p - (intptr_t)e) / sizeof(wchar_t);
						
						if (len > 0) {
							m_ext = memAlloc(wchar_t, m_ext, (len + 1) * sizeof(wchar_t));
							memcpy(m_ext, e, len * sizeof(wchar_t));
							m_ext[len] = 0;
						}
					}
				}
				
				s = p;
				break;
			}
			p++;
		} while (true);
		
		// Query
		if (s[0] == L'?') {
			s++;
			
			// Query
			p = s;
			do {
				ch = *p;
				if ((ch == L'#') || (ch == 0)) {
					int len = ((intptr_t)p - (intptr_t)s) / sizeof(wchar_t);
					
					if (len > 0) {
						m_query = memAlloc(wchar_t, m_query, (len + 1) * sizeof(wchar_t));
						memcpy(m_query, s, len * sizeof(wchar_t));
						m_query[len] = 0;
					}
					
					s = p;
					break;
				}
				p++;
			} while (true);
		}

		// Anchor
		if (s[0] == L'#') {
			s++;
			
			// Anchor
			p = s;
			do {
				ch = *p;
				if (ch == 0) {
					int len = ((intptr_t)p - (intptr_t)s) / sizeof(wchar_t);
					
					if (len > 0) {
						m_anchor = memAlloc(wchar_t, m_anchor, (len + 1) * sizeof(wchar_t));
						memcpy(m_anchor, s, len * sizeof(wchar_t));
						m_anchor[len] = 0;
					}
					
					s = p;
					break;
				}
				p++;
			} while (true);
		}

		if ((m_scheme != NULL) && (m_port != NULL)) {
			if (wcscmp(m_scheme, L"https") == 0) {
				if (wcscmp(m_port, L"443") == 0) {
					memFree(m_port);
					m_port = NULL;
				}
			} else if (wcscmp(m_scheme, L"http") == 0) {
				if (wcscmp(m_port, L"80") == 0) {
					memFree(m_port);
					m_port = NULL;
				}
			}
		}
	}
}

URI::~URI() {
	if (_uri != NULL) {
		memDelete(_uri);
		_uri = NULL;
	}
	if (_absolutePath != NULL) {
		memDelete(_absolutePath);
		_absolutePath = NULL;
	}
	if (_fullPath != NULL) {
		memDelete(_fullPath);
		_fullPath = NULL;
	}
	if (_scheme != NULL) {
		memDelete(_scheme);
		_scheme = NULL;
	}
	if (_server != NULL) {
		memDelete(_server);
		_server = NULL;
	}
	
	if (m_scheme != NULL) {
		memFree(m_scheme);
		m_scheme = NULL;
	}
	if (m_user != NULL) {
		memFree(m_user);
		m_user = NULL;
	}
	if (m_password != NULL) {
		memFree(m_password);
		m_password = NULL;
	}
	if (m_host != NULL) {
		memFree(m_host);
		m_host = NULL;
	}
	if (m_port != NULL) {
		memFree(m_port);
		m_port = NULL;
	}
	if (m_path != NULL) {
		memFree(m_path);
		m_path = NULL;
	}
	if (m_file != NULL) {
		memFree(m_file);
		m_file = NULL;
	}
	if (m_ext != NULL) {
		memFree(m_ext);
		m_ext = NULL;
	}
	if (m_query != NULL) {
		memFree(m_query);
		m_query = NULL;
	}
	if (m_anchor != NULL) {
		memFree(m_anchor);
		m_anchor = NULL;
	}
}

const String& URI::scheme() {
	if (_scheme == NULL) {
		_scheme = memNew(_scheme, String());
		if (m_scheme != NULL) {
			_scheme->concat(m_scheme);
			_scheme->concat(L"://");
		}
	}
	return *_scheme;
}

const String& URI::server() {
	if (_server == NULL) {
		_server = memNew(_server, String());
		if ((m_user != NULL) || (m_password != NULL)) {
			if (m_user != NULL) {
				_server->concat(m_user);
			}
			_server->concat(L":");
			if (m_password != NULL) {
				_server->concat(m_password);
			}
			_server->concat(L"@");
		}
		if (m_host != NULL) {
			_server->concat(m_host);
		}
		if (m_port != NULL) {
			_server->concat(L":");
			_server->concat(m_port);
		}
	}
	return *_server;
}


const String& URI::fullPath() {
	if (_fullPath == NULL) {
		_fullPath = memNew(_fullPath, String());
		_fullPath->concat(scheme());
		_fullPath->concat(server());
		if (m_path != NULL) {
			_fullPath->concat(m_path);
		} else if (_fullPath->length != 0) {
			_fullPath->concat(L"/");
		}
		if (m_file != NULL) {
			_fullPath->concat(m_file);
		}
		uint32_t len = _fullPath->length;
		if (len > 0) {
			if (_fullPath->charAt(len - 1) == L'/') {
				_fullPath->length = len - 1;
			}
		}
	}
	return *_fullPath;
}

const String& URI::absolutePath(const wchar_t* basePath) {
	if ((_absolutePath == NULL) || (basePath != NULL)) {
		if (_absolutePath != NULL) {
			memDelete(_absolutePath);
			_absolutePath = NULL;
		}
		_absolutePath = memNew(_absolutePath, String());
		
		if ((m_scheme == NULL) || (m_host == NULL)) {
			String path;
			if (m_path != NULL) {
				if (m_path[0] == L'/') {
					path = (m_path + 1);
				} else {
					path = m_path;
				}
				uint32_t len = path.length;
				if (len > 0) {
					if (path.charAt(len - 1) == L'/') {
						path.length = len - 1;
					}
				}
			}
			
			if (basePath != NULL) {
				_absolutePath->concat(basePath);
			} else {
				_absolutePath->concat(this->basePath);
			}
			uint32_t len = _absolutePath->length;
			if (len > 0) {
				if (_absolutePath->charAt(len - 1) == L'/') {
					_absolutePath->length = len - 1;
				}
			}
			if (path.length != 0) {
				_absolutePath->concat(L"/");
				_absolutePath->concat(path);
			}
		} else {
			_absolutePath->operator=(fullPath());
		}
	}
	return *_absolutePath;
}

const String& URI::uri() {
	if (_uri == NULL) {
		_uri = memNew(_uri, String());
		_uri->concat(absolutePath());
		_uri->concat(L"/");
		if (m_file != NULL) {
			_uri->concat(m_file);
		}
		if (m_query != NULL) {
			_uri->concat(L"?");
			_uri->concat(m_query);
		}
		if (m_anchor != NULL) {
			_uri->concat(L"#");
			_uri->concat(m_anchor);
		}
	}
	return *_uri;
}

