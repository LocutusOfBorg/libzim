/*
 * Copyright (C) 2009 Tommi Maekitalo
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * is provided AS IS, WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, and
 * NON-INFRINGEMENT.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 */

#ifndef ZIM_WRITER_DIRENT_H
#define ZIM_WRITER_DIRENT_H

#include "cluster.h"

#include "debug.h"

namespace zim
{
  struct DirectInfo {
    cluster_index_t clusterNumber;
    blob_index_t     blobNumber;
  };

  struct RedirectInfo {
    article_index_t redirectIndex;
  };

  union DirentInfo {
    DirectInfo d;
    RedirectInfo r;
  };

  namespace writer
  {
    class Dirent
    {
        static const uint16_t redirectMimeType = 0xffff;
        static const uint16_t linktargetMimeType = 0xfffe;
        static const uint16_t deletedMimeType = 0xfffd;
        static const uint32_t version = 0;

        uint16_t mimeType;
        DirentInfo info {};
        char ns;
        std::string title;
        std::string url;
        std::string parameter;
        Cluster* cluster = nullptr;
        std::string aid;
        std::string redirectAid;
        article_index_t idx = article_index_t(0);

      public:
        Dirent()
          : mimeType(0),
            ns('\0'),
            title(),
            url(),
            parameter(),
            aid(),
            redirectAid()
        {
          info.d.clusterNumber = cluster_index_t(0);
          info.d.blobNumber = blob_index_t(0);
        }

        Dirent(const std::string& aid_)
          : Dirent()
          { aid = aid_; }

        Dirent(char ns_, const std::string& url_ )
          : Dirent()
          { ns = ns_; url = url_; }

        char getNamespace() const               { return ns; }
        const std::string& getTitle() const     { return title.empty() ? url : title; }
        void setTitle(const std::string& title_) { title = title_; }
        const std::string& getUrl() const       { return url; }
        void setUrl(char ns_, const std::string& url_) {
          ns = ns_;
          url = url_;
        }
        const std::string& getParameter() const { return parameter; }
        void setParameter(const std::string& parameter_) {
          parameter = parameter_;
        }

        uint32_t getVersion() const            { return version; }

        void setAid(const std::string&  aid_)      { aid = aid_; }
        const std::string& getAid() const          { return aid; }

        void setRedirectAid(const std::string&  aid_)     { redirectAid = aid_; }
        const std::string& getRedirectAid() const         { return redirectAid; }
        void setRedirect(article_index_t idx) {
          info.r.redirectIndex = idx;
          mimeType = redirectMimeType;
        }
        article_index_t getRedirectIndex() const      { return isRedirect() ? info.r.redirectIndex : article_index_t(0); }

        void setMimeType(uint16_t mime)
        {
          mimeType = mime;
        }

        void setLinktarget()
        {
          ASSERT(mimeType, ==, 0);
          mimeType = linktargetMimeType;
        }

        void setDeleted()
        {
          ASSERT(mimeType, ==, 0);
          mimeType = deletedMimeType;
        }


        void setIdx(article_index_t idx_)      { idx = idx_; }
        article_index_t getIdx() const         { return idx; }


        void setCluster(zim::writer::Cluster* _cluster)
        {
          ASSERT(isArticle(), ==, true);
          cluster = _cluster;
          info.d.blobNumber = _cluster->count();
        }

        cluster_index_t getClusterNumber() const {
          return cluster ? cluster->getClusterIndex() : info.d.clusterNumber;
        }
        blob_index_t  getBlobNumber() const {
          return isRedirect() ? blob_index_t(0) : info.d.blobNumber;
        }

        bool isRedirect() const                 { return mimeType == redirectMimeType; }
        bool isLinktarget() const               { return mimeType == linktargetMimeType; }
        bool isDeleted() const                  { return mimeType == deletedMimeType; }
        bool isArticle() const                  { return !isRedirect() && !isLinktarget() && !isDeleted(); }
        uint16_t getMimeType() const            { return mimeType; }
        size_t getDirentSize() const
        {
          size_t ret = (isRedirect() ? 12 : 16) + url.size() + parameter.size() + 2;
          if (title != url)
            ret += title.size();
          return ret;
        }

        void setArticle(uint16_t mimeType_, cluster_index_t clusterNumber_, blob_index_t blobNumber_)
        {
          ASSERT(mimeType, ==, 0);
          mimeType = mimeType_;
          info.d.clusterNumber = clusterNumber_;
          info.d.blobNumber = blobNumber_;
        }



        friend bool compareUrl(const Dirent* d1, const Dirent* d2);
        friend inline bool compareTitle(const Dirent* d1, const Dirent* d2);
        friend inline bool compareAid(const Dirent* d1, const Dirent* d2);
    };


    inline bool compareUrl(const Dirent* d1, const Dirent* d2)
    {
      return d1->ns < d2->ns
        || (d1->ns == d2->ns && d1->url < d2->url);
    }
    inline bool compareTitle(const Dirent* d1, const Dirent* d2)
    {
      return d1->ns < d2->ns
        || (d1->ns == d2->ns && d1->getTitle() < d2->getTitle());
    }
    inline bool compareAid(const Dirent* d1, const Dirent* d2)
    {
      return d1->aid < d2->aid;
    }

    std::ostream& operator<< (std::ostream& out, const Dirent& d);

  }
}

#endif // ZIM_WRITER_DIRENT_H

