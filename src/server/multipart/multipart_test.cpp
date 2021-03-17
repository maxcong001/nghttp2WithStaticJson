

#include "multipart_reader.hpp"




  content_type get_content_type() {
    if (req_.is_chunked())
      return content_type::chunked;

    auto content_type = req_.get_header_value("content-type");
    if (!content_type.empty()) {
      if (content_type.find("application/x-www-form-urlencoded") !=
          std::string_view::npos) {
        return content_type::urlencoded;
      } else if (content_type.find("multipart/form-data") !=
                 std::string_view::npos) {
        auto size = content_type.find("=");
        auto bd = content_type.substr(size + 1, content_type.length() - size);
        if (bd[0] == '"' && bd[bd.length() - 1] == '"') {
          bd = bd.substr(1, bd.length() - 2);
        }
        std::string boundary(bd.data(), bd.length());
        multipart_parser_.set_boundary("\r\n--" + std::move(boundary));
        return content_type::multipart;
      } else if (content_type.find("application/octet-stream") !=
                 std::string_view::npos) {
        return content_type::octet_stream;
      } else {
        return content_type::string;
      }
    }

    return content_type::unknown;
  }

 void init_multipart_parser() {
    multipart_parser_.on_part_begin = [this](const multipart_headers &headers) {
      req_.set_multipart_headers(headers);
      auto filename = req_.get_multipart_field_name("filename");
      is_multi_part_file_ = req_.is_multipart_file();
      if (filename.empty() && is_multi_part_file_) {
        req_.set_state(data_proc_state::data_error);
        res_.set_status_and_content(status_type::bad_request, "mutipart error");
        return;
      }
      if (is_multi_part_file_) {
        auto ext = get_extension(filename);
        try {
          auto tp = std::chrono::high_resolution_clock::now();
          auto nano = tp.time_since_epoch().count();
          std::string name = static_dir_ + "/" + std::to_string(nano) +
                             std::string(ext.data(), ext.length()) + "_ing";
          if (multipart_begin_) {
            multipart_begin_(req_, name);
            name = static_dir_ + "/" + name;
          }

          req_.open_upload_file(name);
        } catch (const std::exception &ex) {
          req_.set_state(data_proc_state::data_error);
          res_.set_status_and_content(status_type::internal_server_error,
                                      ex.what());
          return;
        }
      } else {
        auto key = req_.get_multipart_field_name("name");
        req_.save_multipart_key_value(std::string(key.data(), key.size()), "");
      }
    };
    multipart_parser_.on_part_data = [this](const char *buf, size_t size) {
      if (req_.get_state() == data_proc_state::data_error) {
        return;
      }
      if (is_multi_part_file_) {
        req_.write_upload_data(buf, size);
      } else {
        auto key = req_.get_multipart_field_name("name");
        req_.update_multipart_value(std::move(key), buf, size);
      }
    };
    multipart_parser_.on_part_end = [this] {
      if (req_.get_state() == data_proc_state::data_error)
        return;
      if (is_multi_part_file_) {
        req_.close_upload_file();
        auto pfile = req_.get_file();
        if (pfile) {
          auto old_name = pfile->get_file_path();
          auto pos = old_name.rfind("_ing");
          if (pos != std::string::npos) {
            pfile->rename_file(old_name.substr(0, old_name.length() - 4));
          }
        }
      }
    };
    multipart_parser_.on_end = [this] {
      if (req_.get_state() == data_proc_state::data_error)
        return;
      req_.handle_multipart_key_value();
      // call_back();
    };
  }