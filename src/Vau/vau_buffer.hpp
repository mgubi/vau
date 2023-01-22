//
//  vau_buffer.hpp
//  VauQt
//
//  Created by Massimiliano Gubinelli on 26/12/2022.
//

#ifndef VAU_BUFFER_H
#define VAU_BUFFER_H

#include "new_data.hpp"
#include "new_document.hpp"
#include "url.hpp"
#include "tm_timer.hpp"
#include "link.hpp"


/******************************************************************************
* The buffer class
******************************************************************************/

class new_buffer;
class new_buffer_rep: public concrete_struct {
public:
  url name;               // full name
  url master;             // base name for linking and navigation
  string fm;              // buffer format
  string title;           // buffer title (for menus)
  bool read_only;         // buffer is read only?
  bool secure;            // is the buffer secure?
  int last_save;          // last time that the buffer was saved
  time_t last_visit;      // time that the buffer was visited last

  inline new_buffer_rep (url name2):
    name (name2), master (name2),
    fm ("texmacs"), title (as_string (tail (name))),
    read_only (false), secure (is_secure (name2)),
    last_save (- (int) (((unsigned int) (-1)) >> 1)),
    last_visit (texmacs_time ()) {}
};

class new_buffer;
class new_buffer {
CONCRETE(new_buffer);
  inline new_buffer (url name): rep (tm_new<new_buffer_rep> (name)) {}
};
//CONCRETE_CODE(new_buffer);

inline new_buffer::new_buffer (const new_buffer& x):
  rep(x.rep) { INC_COUNT (this->rep); }
inline new_buffer::~new_buffer () { DEC_COUNT (this->rep); }
inline new_buffer_rep* new_buffer::operator -> () {
  return rep; }
inline new_buffer& new_buffer::operator = (new_buffer x) {
  INC_COUNT (x.rep); DEC_COUNT (this->rep);
  this->rep=x.rep; return *this; }


/******************************************************************************
* vau_buffer
******************************************************************************/

class vau_buffer_rep;
typedef vau_buffer_rep* vau_buffer;

class vau_buffer_rep {
public:
  new_buffer buf;         // file related information
  new_data data;          // data associated to document
//  array<tm_view> vws;     // views attached to buffer
  vau_buffer prj;          // buffer which corresponds to the project
  path rp;                // path to the document's root in the_et
  link_repository lns;    // global links
  bool notify;            // notify modifications to scheme

  inline vau_buffer_rep (url name):
    buf (name), data (),
    prj (NULL),
  rp (new_document ()),
  notify (false) {}

  inline ~vau_buffer_rep () {
    delete_document (rp);
  }

  void attach_notifier ();
  bool needs_to_be_saved ();
  bool needs_to_be_autosaved ();
};

inline vau_buffer nil_buffer () { return (vau_buffer) NULL; }
inline bool is_nil (vau_buffer buf) { return buf == NULL; }

vau_buffer concrete_buffer (url name);
vau_buffer concrete_buffer_insist (url name);

void set_buffer_tree (url name, tree doc);
bool buffer_load (url name);

#endif /* vau_buffer_hpp */
